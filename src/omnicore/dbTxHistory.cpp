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
int64_t COmniTxHistory::CountRecords()const
{
    // No faster way to count than to iterate - "There is no way to implement Count more efficiently inside leveldb than outside."
    int64_t count = 0;
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
	int64_t index = GetTop();
	uint32_t historyBlock = -1 ;
	while(historyBlock >= uint32_t(block)){
		std::string key_index = strprintf("%lld", index--);
		std::string strValue;
		leveldb::Status status = pdb->Get(readoptions, key_index, &strValue);
		if (status.IsNotFound()) {
			return ; // fee distribution not found, return empty set
		}
		std::vector<std::string> vHistoryDetail;
		boost::split(vHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
		if(vHistoryDetail.size() !=2)
			return;
		historyBlock = boost::lexical_cast<int>(vHistoryDetail[1]);
		if (historyBlock >= block) {
			pdb->Delete(writeoptions, key_index);
			pdb->Delete(writeoptions, vHistoryDetail[0]);
		}
	}
}

bool COmniTxHistory::PutHistory(const std::string& key, int nBlock, const std::string& history)
{
	assert(pdb);
	int64_t index = GetTop() +1;
	std::string key_index = strprintf("%lld", index);
	std::string value = strprintf("%s:%lld", key, nBlock);
	leveldb::Status status = pdb->Put(writeoptions, key_index, value);

	value = strprintf("%d:%s:%lld", nBlock, history, index);
	status = pdb->Put(writeoptions, key, value);
    if (msc_debug_fees) PrintToLog("Added fee distribution to feeCacheHistory - key=%s value=%s [%s]\n", key, value, status.ToString());
	return true;
}

std::string COmniTxHistory::GetHistory(const std::string& key)
{
	//const std::string key = strprintf("%d", index);
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);
	if (status.IsNotFound()) {
        return ""; // fee distribution not found, return empty set
    }
    std::vector<std::string> vHistoryDetail;
    boost::split(vHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
	if(vHistoryDetail.size() !=3)
		return "";
	std::vector<unsigned char> vecRet = ParseHex(vHistoryDetail[1]);
	//if(vecRet)
	return std::string(vecRet.begin(), vecRet.end());
}

std::string COmniTxHistory::GetHistory(int64_t index)
{
	const std::string key = strprintf("%lld", index);
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);

	if (status.IsNotFound()) {
        return ""; // fee distribution not found, return empty set
    }
    std::vector<std::string> vHistoryDetail;
    boost::split(vHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
	if(vHistoryDetail.size() !=2)
		return "";

	return GetHistory(vHistoryDetail[0]);
}

std::string COmniTxHistory::GetEndHistory()
{
	return GetHistory(GetTop());
}

int64_t COmniTxHistory::GetTop()const
{
	assert(pdb);

	int64_t left = 0;
	int64_t right = CountRecords();
	int64_t last_index = -1;
	int64_t index = 0;
	std::string hash;
	while (left < right)
	{
		index = (left + right) >>1;
		if(IsExist(index))
		{
			last_index = index;
			left = index + 1;
		}else{
			right = index - 1;
		}
	}
	index = (left + right) >>1;
	if(!IsExist(index)) return last_index;
	else return last_index > index ? last_index : index;
}

bool COmniTxHistory::IsExist(int64_t index)const
{
	assert(pdb);

    const std::string key = strprintf("%lld", index);
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);
	return !status.IsNotFound();
}

/****************************************************************************/

COmniPaymentTxHistory::COmniPaymentTxHistory(const boost::filesystem::path& path, bool fWipe):
	COmniTxHistory(path, fWipe)
{
//    leveldb::Status status = Open(path, fWipe);
//    PrintToConsole("Loading COmniPaymentTxHistory database: %s\n", status.ToString());
}

COmniPaymentTxHistory::~COmniPaymentTxHistory()
{
    if (msc_debug_fees) PrintToLog("COmniPaymentTxHistory closed\n");
}