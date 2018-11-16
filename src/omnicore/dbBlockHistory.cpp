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

COmniBlockHistory::COmniBlockHistory(const boost::filesystem::path& path, bool fWipe)
{
    leveldb::Status status = Open(path, fWipe);
    PrintToConsole("Loading Block history database: %s\n", status.ToString());
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
void COmniBlockHistory::RollBackHistory(int block, int top)
{
#ifdef _BLOCK_NIN_
    std::set<int> sDistributions;
    leveldb::Iterator* it = NewIterator();
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::string strValue = it->value().ToString();
        std::string strKey = it->key().ToString();
        std::vector<std::string> vBlockHistoryDetail;
        boost::split(vBlockHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
        if (3 != vBlockHistoryDetail.size()) {
            PrintToLog("ERROR: vBlockHistoryDetail has unexpected number of elements: %d !\n", vBlockHistoryDetail.size());
            continue; // bad data
        }
        int historyBlock = boost::lexical_cast<int>(vBlockHistoryDetail[0]);
        if (historyBlock >= block) {
            PrintToLog("%s() deleting from block history DB: %s %s\n", __FUNCTION__, strKey, strValue);
            pdb->Delete(writeoptions, strKey);
        }
    }
    delete it;
#else
    assert(pdb);

	int height = -1;
	std::string hash;
	int index = block;
	GetBlockHistory(index, height, hash);
	while (index <= top )
	{
		if (height >= block) {
			const std::string key = strprintf("%d", height);
            pdb->Delete(writeoptions, key);
			pdb->Delete(writeoptions, hash);
        }
		index++;
		GetBlockHistory(index, height, hash);
	}
#endif
}

bool COmniBlockHistory::GetBlockHistory(int index, int& height, std::string& hash, int64_t* pTime)
{
#ifdef _BLOCK_NIN_
    leveldb::Iterator* it = NewIterator();
	std::vector<std::string> vHistoryDetail;
    for(it->SeekToFirst(); it->Valid(); it->Next()) {
		boost::split(vHistoryDetail, it->value().ToString(), boost::is_any_of(":"), boost::token_compress_on);
		if(vHistoryDetail.size() != 3)
			break;
		height = atoi(vHistoryDetail[0].c_str());
		hash = vHistoryDetail[1];

		if(index == height){
			if(pTime) *pTime = atoi64(vHistoryDetail[2].c_str());
			return true;
		}
    }
    delete it;
	height = 0;
	hash.clear();
	return false;
#else
	height = -1;
	hash.clear();
	const std::string key = strprintf("%d", index);
    std::set<feeHistoryItem> sFeeHistoryItems;
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);
	if (status.IsNotFound()) {
        return false; // fee distribution not found, return empty set
    }
    std::vector<std::string> vHistoryDetail;
    boost::split(vHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
	if(vHistoryDetail.size() != 3)
		return false;

	height = atoi(vHistoryDetail[0].c_str());
	hash = vHistoryDetail[1];
	if(pTime) *pTime = atoi64(vHistoryDetail[2].c_str());
	return true;
#endif
}

bool COmniBlockHistory::GetBlockHistory(const std::string& key, int& height, std::string& hash, int64_t* pTime)
{    std::set<feeHistoryItem> sFeeHistoryItems;
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);
	if (status.IsNotFound()) {
        return false; // fee distribution not found, return empty set
    }
    std::vector<std::string> vHistoryDetail;
    boost::split(vHistoryDetail, strValue, boost::is_any_of(":"), boost::token_compress_on);
	if(vHistoryDetail.size() != 3)
		return false;

	height = atoi(vHistoryDetail[0].c_str());
	hash = vHistoryDetail[1];
	if(pTime) *pTime = atoi64(vHistoryDetail[2].c_str());
	return true;
}

bool COmniBlockHistory::GetEndHistory(int& height, std::string& hash, int64_t* pTime)
{
#ifdef _BLOCK_NIN_
	leveldb::Iterator* it = NewIterator();
	std::vector<std::string> vHistoryDetail;
	int index = 0;
	height = 0;
    for(it->SeekToFirst(); it->Valid(); it->Next()) {
		boost::split(vHistoryDetail, it->value().ToString(), boost::is_any_of(":"), boost::token_compress_on);
		if(vHistoryDetail.size() != 3)
			break;
		height = atoi(vHistoryDetail[0].c_str());
		if(index < height){
			hash = vHistoryDetail[1];
			if(pTime) *pTime = atoi64(vHistoryDetail[2].c_str());
			index = height;
		}
    }
	
    delete it;
	if(height < index && index > 0){
		height = index;
		return true;
	}
	height = 0;
	hash.clear();
	return false;
#else
	return GetBlockHistory(GetTopBlock(), height, hash, pTime);
#endif
}

bool COmniBlockHistory::PutBlockHistory(int height, const std::string& hash, int64_t blockTime)
{
#ifdef _BLOCK_NIN_
	assert(pdb);
	std::string value = strprintf("%d:%s:%lld", height, hash, blockTime);
	leveldb::Status status = pdb->Put(writeoptions, hash, value);
	if (msc_debug_fees) PrintToLog("Added fee distribution to feeCacheHistory - key=%s value=%s [%s]\n", hash, value, status.ToString());
#else
	assert(pdb);
	std::string key = strprintf("%d", height);
	std::string value = strprintf("%d:%s:%lld", height, hash, blockTime);
	leveldb::Status status = pdb->Put(writeoptions, key, value);
	status = pdb->Put(writeoptions, hash, value);
	if (msc_debug_fees) PrintToLog("Added fee distribution to feeCacheHistory - key=%s value=%s [%s]\n", key, value, status.ToString());
#endif
	return true;
}

int COmniBlockHistory::GetRightNilBlock(int left)
{
	int right = left;
	if(right <= 0)
	{
		right = 1;
	}

	int curHeight = -1;
	std::string hash;
	GetBlockHistory(right, curHeight, hash);

	while(curHeight != -1)
	{
		right = right << 1;
		GetBlockHistory(right, curHeight, hash);
	}
	return right;
}

int COmniBlockHistory::GetLeftTopBlock()
{
	int left = 0;
    leveldb::Iterator* it = NewIterator();
    for(it->SeekToFirst(); it->Valid(); it->Next()) {

		std::vector<std::string> vHistoryDetail;
		boost::split(vHistoryDetail, it->value().ToString(), boost::is_any_of(":"), boost::token_compress_on);
		if(vHistoryDetail.size() != 3)
		{
			left = 0;
			break;
		}
		left = atoi(vHistoryDetail[0].c_str());
		break;
    }
    delete it;
	return left;
}

int COmniBlockHistory::GetTopBlock()
{
#ifdef _BLOCK_NIN_
	leveldb::Iterator* it = NewIterator();
	std::vector<std::string> vHistoryDetail;
	int top = 0;
	int height = 0;
    for(it->SeekToFirst(); it->Valid(); it->Next()) {
		boost::split(vHistoryDetail, it->value().ToString(), boost::is_any_of(":"), boost::token_compress_on);
		if(vHistoryDetail.size() != 3)
			break;
		height = atoi(vHistoryDetail[0].c_str());
		if(top < height){
			top = height;
		}
    }
    delete it;
	return top;
#else
	assert(pdb);

	int left = GetLeftTopBlock();
	int right = GetRightNilBlock(left);
	int curHeight = -1;
	int index = 0;
	std::string hash;
	while (left < right)
	{
		index = (left + right) >>1;
		GetBlockHistory(index, curHeight, hash);
		if(curHeight == -1)
		{
			right = index - 1;
		}else{
			left = index + 1;
		}
	}
	GetBlockHistory(left, curHeight, hash);
	if(curHeight == -1)
	{
		return index;
	}else{
		return curHeight;
	}
#endif
}