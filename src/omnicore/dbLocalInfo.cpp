/**
 * @file dbfees.cpp
 *
 * This file contains code for handling Omni fees.
 */

#include "omnicore/dbLocalInfo.h"

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

CLocalBlockInfo::CLocalBlockInfo(const boost::filesystem::path& path, bool fWipe)
{
    leveldb::Status status = Open(path, fWipe);
    PrintToConsole("Loading LocalBlockInfo database: %s\n", status.ToString());
}

CLocalBlockInfo::~CLocalBlockInfo()
{
    if (msc_debug_fees) PrintToLog("CLocalBlockInfo closed\n");
}
    
// Show Fee History DB statistics
void CLocalBlockInfo::printStats()
{
    PrintToConsole("COmniTxHistory stats: nWritten= %d , nRead= %d\n", nWritten, nRead);
}

// Show Fee History DB records
void CLocalBlockInfo::printAll()
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
int64_t CLocalBlockInfo::CountRecords()const
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
void CLocalBlockInfo::RollBackHistory(int64_t block)
{
    assert(pdb);
	int count = 0;
    leveldb::Iterator* it = NewIterator();
    for(it->SeekToFirst(); it->Valid(); it->Next()) {
        ++count;
		int64_t key = atoi64(it->key().ToString().c_str());
		if(key >= block)
		{
			pdb->Delete(writeoptions, it->key());
		}
    }
    delete it;
}

bool CLocalBlockInfo::PutInfo(int64_t height, const std::string& info)
{
	assert(pdb);
	std::string key = strprintf("%lld", height);
	leveldb::Status status = pdb->Put(writeoptions, key, info);
	return true;
}

std::string CLocalBlockInfo::GetBlockInfo(int64_t index)
{
	const std::string key = strprintf("%lld", index);
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);

	if (status.IsNotFound()) {
        return ""; // fee distribution not found, return empty set
    }
	return strValue;
}

// 
void CLocalBlockInfo::Delete(int64_t block)
{
	const std::string key = strprintf("%lld", block);
	pdb->Delete(writeoptions, key);
}

/************************************************************************************/

CLocalTxInfo::CLocalTxInfo(const boost::filesystem::path& path, bool fWipe)
{
    leveldb::Status status = Open(path, fWipe);
    PrintToConsole("Loading LocalTxInfo database: %s\n", status.ToString());
}

CLocalTxInfo::~CLocalTxInfo()
{
    if (msc_debug_fees) PrintToLog("LocalTxInfo closed\n");
}
    
// Show Fee History DB statistics
void CLocalTxInfo::printStats()
{
    PrintToConsole("CLocalTxInfo stats: nWritten= %d , nRead= %d\n", nWritten, nRead);
}

// Show Fee History DB records
void CLocalTxInfo::printAll()
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
int64_t CLocalTxInfo::CountRecords()const
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
void CLocalTxInfo::RollBackHistory(int64_t block)
{
    assert(pdb);
	int count = 0;
    leveldb::Iterator* it = NewIterator();
    for(it->SeekToFirst(); it->Valid(); it->Next()) {
        ++count;
		int64_t key = atoi64(it->key().ToString().c_str());
		if(key >= block)
		{
			pdb->Delete(writeoptions, it->key());
		}
    }
    delete it;
}

bool CLocalTxInfo::PutData(int64_t nBlock, const std::string& data)
{
	assert(pdb);
	std::string key = strprintf("%lld", nBlock);

	std::string info =  GetTxInfo(nBlock);
	leveldb::Status status = pdb->Put(writeoptions, key, info.empty() ? data : info + ":" + data);

	return true;
}

std::string CLocalTxInfo::GetTxInfo(int64_t index, std::vector<std::string>* vecTxList)
{
	const std::string key = strprintf("%lld", index);
    std::string strValue;
    leveldb::Status status = pdb->Get(readoptions, key, &strValue);

	if (status.IsNotFound()) {
        return ""; // fee distribution not found, return empty set
    }
	if(vecTxList)
		boost::split(*vecTxList, strValue, boost::is_any_of(":"), boost::token_compress_on);
	return strValue;
}

// 
void CLocalTxInfo::Delete(int64_t block)
{
	const std::string key = strprintf("%lld", block);
	pdb->Delete(writeoptions, key);
}