/**
 * @file dbfees.cpp
 *
 * This file contains code for handling Omni fees.
 */

#include "omnicore/dbBlockHistory.h"

#include "omnicore/log.h"
#include "omnicore/omnicore.h"
#include "omnicore/rules.h"
#include "omnicore/sp.h"
#include "omnicore/sto.h"

#include "main.h"

#include "leveldb/db.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <stdint.h>

#include <limits>
#include <map>
#include <string>
#include <vector>

using namespace mastercore;

COmniBlockHistory::COmniBlockHistory(const boost::filesystem::path& path, bool fWipe)
{
    leveldb::Status status = Open(path, fWipe);
    PrintToConsole("Loading Tx history database: %s\n", status.ToString());
}

COmniBlockHistory::~COmniBlockHistory()
{
    if (msc_debug_fees) PrintToLog("COmniTxHistory closed\n");
}
    
// Show Fee History DB statistics
void COmniBlockHistory::printStats()
{
    PrintToConsole("COmniTxHistory stats: nWritten= %d , nRead= %d\n", nWritten, nRead);
}

// Show Fee History DB records
void COmniBlockHistory::printAll()
{
    int count = 0;
    leveldb::Iterator* it = NewIterator();
    for(it->SeekToFirst(); it->Valid(); it->Next()) {
        ++count;
        PrintToConsole("entry #%8d= %s-%s\n", count, it->key().ToString(), it->value().ToString());
        PrintToLog("entry #%8d= %s-%s\n", count, it->key().ToString(), it->value().ToString());
    }
    delete it;
}

// Count Fee History DB records
int COmniBlockHistory::CountRecords()
{
    // No faster way to count than to iterate - "There is no way to implement Count more efficiently inside leveldb than outside."
    int count = 0;
    leveldb::Iterator* it = NewIterator();
    for(it->SeekToFirst(); it->Valid(); it->Next()) {
        ++count;
    }
    delete it;
    return count;
}

// Roll back history in event of reorg, block is inclusive
void COmniBlockHistory::RollBackHistory(int block)
{
    assert(pdb);

    std::set<int> sDistributions;
    leveldb::Iterator* it = NewIterator();
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::string strValue = it->value().ToString();
        std::string strKey = it->key().ToString();
        std::vector<std::string> vFeeHistoryDetail;
        boost::split(vFeeHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
        if (2 != vFeeHistoryDetail.size()) {
            PrintToLog("ERROR: vFeeHistoryDetail has unexpected number of elements: %d !\n", vFeeHistoryDetail.size());
            continue; // bad data
        }
        int historyBlock = boost::lexical_cast<int>(vFeeHistoryDetail[0]);
        if (historyBlock >= block) {
            PrintToLog("%s() deleting from fee history DB: %s %s\n", __FUNCTION__, strKey, strValue);
            pdb->Delete(writeoptions, strKey);
        }
    }
    delete it;
}

// Retrieve fee distributions for a property
std::set<int> COmniBlockHistory::GetDistributionsForProperty(const uint32_t &propertyId)
{
    assert(pdb);

    std::set<int> sDistributions;
    leveldb::Iterator* it = NewIterator();
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::string strValue = it->value().ToString();
        std::vector<std::string> vFeeHistoryDetail;
        boost::split(vFeeHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
        if (4 != vFeeHistoryDetail.size()) {
            PrintToConsole("ERROR: vFeeHistoryDetail has unexpected number of elements: %d !\n", vFeeHistoryDetail.size());
            printAll();
            continue; // bad data
        }
        uint32_t prop = boost::lexical_cast<uint32_t>(vFeeHistoryDetail[1]);
        if (prop == propertyId) {
            std::string key = it->key().ToString();
            int id = boost::lexical_cast<int>(key);
            sDistributions.insert(id);
        }
    }
    delete it;
    return sDistributions;
}

// Populate data about a fee distribution
bool COmniBlockHistory::GetDistributionData(int id, uint32_t *propertyId, int *block, int64_t *total)
{
    assert(pdb);

    const std::string key = strprintf("%d", id);
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);
    if (status.IsNotFound()) {
        return false; // fee distribution not found
    }
    assert(status.ok());
    std::vector<std::string> vFeeHistoryDetail;
    boost::split(vFeeHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
    if (4 != vFeeHistoryDetail.size()) {
        PrintToConsole("ERROR: vFeeHistoryDetail has unexpected number of elements: %d !\n", vFeeHistoryDetail.size());
        printAll();
        return false; // bad data
    }
    *block = boost::lexical_cast<int>(vFeeHistoryDetail[0]);
    *propertyId = boost::lexical_cast<uint32_t>(vFeeHistoryDetail[1]);
    *total = boost::lexical_cast<int64_t>(vFeeHistoryDetail[2]);
    return true;
}

// Retrieve the recipients for a fee distribution
std::set<feeHistoryItem> COmniBlockHistory::GetFeeDistribution(int id)
{
    assert(pdb);

    const std::string key = strprintf("%d", id);
    std::set<feeHistoryItem> sFeeHistoryItems;
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);
    if (status.IsNotFound()) {
        return sFeeHistoryItems; // fee distribution not found, return empty set
    }
    assert(status.ok());
    std::vector<std::string> vFeeHistoryDetail;
    boost::split(vFeeHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
    if (4 != vFeeHistoryDetail.size()) {
        PrintToConsole("ERROR: vFeeHistoryDetail has unexpected number of elements: %d !\n", vFeeHistoryDetail.size());
        printAll();
        return sFeeHistoryItems; // bad data, return empty set
    }
    std::vector<std::string> vFeeHistoryItems;
    boost::split(vFeeHistoryItems, vFeeHistoryDetail[3], boost::is_any_of(","), boost::token_compress_on);
    for (std::vector<std::string>::iterator it = vFeeHistoryItems.begin(); it != vFeeHistoryItems.end(); ++it) {
        std::vector<std::string> vFeeHistoryItem;
        boost::split(vFeeHistoryItem, *it, boost::is_any_of("="), boost::token_compress_on);
        if (2 != vFeeHistoryItem.size()) {
            PrintToConsole("ERROR: vFeeHistoryItem has unexpected number of elements: %d (raw %s)!\n", vFeeHistoryItem.size(), *it);
            printAll();
            continue;
        }
        int64_t feeHistoryItemAmount = boost::lexical_cast<int64_t>(vFeeHistoryItem[1]);
        sFeeHistoryItems.insert(std::make_pair(vFeeHistoryItem[0], feeHistoryItemAmount));
    }

    return sFeeHistoryItems;
}

// Record a fee distribution
void COmniBlockHistory::RecordFeeDistribution(const uint32_t &propertyId, int block, int64_t total, std::set<feeHistoryItem> feeRecipients)
{
    assert(pdb);

    int count = CountRecords() + 1;
    std::string key = strprintf("%d", count);
    std::string feeRecipientsStr;

    if (!feeRecipients.empty()) {
        for (std::set<feeHistoryItem>::iterator it = feeRecipients.begin(); it != feeRecipients.end(); it++) {
            feeHistoryItem tempRecipient = *it;
            feeRecipientsStr += strprintf("%s=%d,", tempRecipient.first, tempRecipient.second);
        }
        if (feeRecipientsStr.size() > 0) {
            feeRecipientsStr.resize(feeRecipientsStr.size() - 1);
        }
    }

    std::string value = strprintf("%d:%d:%d:%s", block, propertyId, total, feeRecipientsStr);
    leveldb::Status status = pdb->Put(writeoptions, key, value);
    if (msc_debug_fees) PrintToLog("Added fee distribution to feeCacheHistory - key=%s value=%s [%s]\n", key, value, status.ToString());
}


bool COmniBlockHistory::GetBlockHistory(int index, int& height, std::string& hash)
{
	const std::string key = strprintf("%d", index);
    std::set<feeHistoryItem> sFeeHistoryItems;
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);
	if (status.IsNotFound()) {
        return ""; // fee distribution not found, return empty set
    }
    std::vector<std::string> vHistoryDetail;
    boost::split(vHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
	if(vHistoryDetail.size() != 2)
		return "";

	height = atoi(vHistoryDetail[0].c_str());
	hash = vHistoryDetail[1];
	return true;
}

bool COmniBlockHistory::GetBlockHistory(const std::string& key, int& height, std::string& hash)
{
    std::set<feeHistoryItem> sFeeHistoryItems;
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);
	if (status.IsNotFound()) {
        return ""; // fee distribution not found, return empty set
    }
    std::vector<std::string> vHistoryDetail;
    boost::split(vHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
	if(vHistoryDetail.size() != 2)
		return "";

	height = atoi(vHistoryDetail[0].c_str());
	hash = vHistoryDetail[1];
	return true;
}

bool COmniBlockHistory::GetEndHistory(int& height, std::string& hash)
{
	return GetBlockHistory(CountRecords()-1, height, hash);
}

bool COmniBlockHistory::PutBlockHistory(int height, const std::string& hash)
{
	assert(pdb);

//	int lastHeight = -1;
//	std::string lastHash;
	//GetEndHistory(lastHeight, lastHash);
	//if(lastHeight == -1)
	//{
	//	printAll();
	//}
//	if(lastHeight >=0)
//	{
//		if(height != lastHeight + 1)
//		{
//			int tempHeight = -1;
//			std::string tempHash;
//			GetBlockHistory(height, tempHeight, tempHash);
//			PrintToConsole("tempHeight = %d, tempHash = %s, height = %d, hash = %s", tempHeight, tempHash, height, hash);
//		}
//	}
	//if(lastHeight >=height)
	//{
	//	RollBackHistory(height);
	//}
//	int count = CountRecords() + 1;
	std::string key = strprintf("%d", height);
	std::string value = strprintf("%d:%s", height, hash);
	leveldb::Status status = pdb->Put(writeoptions, key, value);
	status = pdb->Put(writeoptions, hash, value);
	if (msc_debug_fees) PrintToLog("Added fee distribution to feeCacheHistory - key=%s value=%s [%s]\n", key, value, status.ToString());
	
	return true;
}