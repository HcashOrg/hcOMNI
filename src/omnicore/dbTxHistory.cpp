/**
 * @file dbfees.cpp
 *
 * This file contains code for handling Omni fees.
 */

#include "omnicore/dbTxHistory.h"

#include "omnicore/log.h"
#include "omnicore/omnicore.h"
#include "omnicore/rules.h"
#include "omnicore/sp.h"
#include "omnicore/sto.h"
#include "utilstrencodings.h"

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

COmniTxHistory::COmniTxHistory(const boost::filesystem::path& path, bool fWipe)
{
    leveldb::Status status = Open(path, fWipe);
    PrintToConsole("Loading Tx history database: %s\n", status.ToString());
}

COmniTxHistory::~COmniTxHistory()
{
    if (msc_debug_fees) PrintToLog("COmniTxHistory closed\n");
}
    
// Show Fee History DB statistics
void COmniTxHistory::printStats()
{
    PrintToConsole("COmniTxHistory stats: nWritten= %d , nRead= %d\n", nWritten, nRead);
}

// Show Fee History DB records
void COmniTxHistory::printAll()
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
int COmniTxHistory::CountRecords()
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
void COmniTxHistory::RollBackHistory(int block)
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
std::set<int> COmniTxHistory::GetDistributionsForProperty(const uint32_t &propertyId)
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
bool COmniTxHistory::GetDistributionData(int id, uint32_t *propertyId, int *block, int64_t *total)
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
std::set<feeHistoryItem> COmniTxHistory::GetFeeDistribution(int id)
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
void COmniTxHistory::RecordFeeDistribution(const uint32_t &propertyId, int block, int64_t total, std::set<feeHistoryItem> feeRecipients)
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

bool COmniTxHistory::PutHistory(const std::string& key, int nBlock, const std::string& history)
{
	assert(pdb);

	std::string value = strprintf("%d:%s", nBlock, history);
	leveldb::Status status = pdb->Put(writeoptions, key, value);
    if (msc_debug_fees) PrintToLog("Added fee distribution to feeCacheHistory - key=%s value=%s [%s]\n", key, value, status.ToString());
	return true;
}

std::string COmniTxHistory::GetHistory(const std::string& key)
{
	//const std::string key = strprintf("%d", index);
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
	std::vector<unsigned char> vecRet = ParseHex(vHistoryDetail[1]);
	//if(vecRet)
	return std::string(vecRet.begin(), vecRet.end());
}

std::string COmniTxHistory::GetHistory(int index)
{
	std::string key;
	int count = 0;
    leveldb::Iterator* it = NewIterator();
    for(it->SeekToFirst(); it->Valid(); it->Next()) {
		if(index == count)
		{
			key = it->key().ToString();
			break;
		}
        ++count;
    }
    delete it;
	if(key.empty())
		return "";
	return GetHistory(key);
}

std::string COmniTxHistory::GetEndHistory()
{
	return GetHistory(CountRecords());
}