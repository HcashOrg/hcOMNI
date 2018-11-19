#ifndef OMNICORE_LOCALINFO_H
#define OMNICORE_LOCALINFO_H

#include "omnicore/dbbase.h"
#include "omnicore/log.h"

#include <boost/filesystem.hpp>

#include <stdint.h>
#include <set>
#include <string>
#include <utility>


/** LevelDB based storage for the LocalBlockInfo.
 */
class CLocalBlockInfo : public CDBBase
{
public:
    CLocalBlockInfo(const boost::filesystem::path& path, bool fWipe);
    virtual ~CLocalBlockInfo();

    /** Show Fee History DB statistics */
    void printStats();
    /** Show Fee History DB records */
    void printAll();

    /** Roll back history in event of reorg */
    void RollBackHistory(int64_t block);
    /** Count Fee History DB records */
    int64_t CountRecords()const;
   
	std::string GetBlockInfo(int64_t index);

	bool PutInfo(int64_t height, const std::string& info);
	void Delete(int64_t block);
};


class CLocalTxInfo : public CDBBase
{
public:
    CLocalTxInfo(const boost::filesystem::path& path, bool fWipe);
    virtual ~CLocalTxInfo();

    /** Show Fee History DB statistics */
    void printStats();
    /** Show Fee History DB records */
    void printAll();

    /** Roll back history in event of reorg */
    void RollBackHistory(int64_t block);
    /** Count Fee History DB records */
    int64_t CountRecords()const;
   
	std::string GetTxInfo(int64_t index, std::vector<std::string>* vecTxList = nullptr);
	bool PutData(int64_t nBlock, const std::string& history);
	void Delete(int64_t block);
};

namespace mastercore
{
	//! LevelDB based storage for the LocalBlockInfo
    extern CLocalBlockInfo* p_localblockinfo;

	//! LevelDB based storage for the LocalBlockInfo
    extern CLocalTxInfo* p_localtxinfo;
}

#endif // OMNICORE_LOCALINFO_H
