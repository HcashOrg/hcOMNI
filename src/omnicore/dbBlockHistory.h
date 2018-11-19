#ifndef OMNICORE_BLOCK_HISTORY_H
#define OMNICORE_BLOCK_HISTORY_H

//#ifndef _BLOCK_NIN_
//#define _BLOCK_NIN_
//#endif

#include "omnicore/dbbase.h"
#include "omnicore/log.h"

#include <boost/filesystem.hpp>

#include <stdint.h>
#include <set>
#include <string>
#include <utility>


/** LevelDB based storage for the TxHistory.
 */
class COmniBlockHistory : public CDBBase
{
public:
    COmniBlockHistory(const boost::filesystem::path& path, bool fWipe);
    virtual ~COmniBlockHistory();

    /** Show Fee History DB statistics */
    void printStats();
    /** Show Fee History DB records */
    void printAll();

    /** Roll back history in event of reorg */
    void RollBackHistory(int block, int top);
    
   
	//key euqal hash
	bool GetBlockHistory(const std::string& key, int& height, std::string& hash, int64_t* pTime = NULL);

	bool GetBlockHistory(int index, int& height, std::string& hash, int64_t* pTime = NULL);
	bool GetEndHistory(int& height, std::string& hash, int64_t* pTime = NULL);
	int GetTopBlock();
	bool PutBlockHistory(int height, const std::string& hash, int64_t blockTime);

private:
	/** Count Fee History DB records */
    int CountRecords();
	int GetRightNilBlock(int left);
	int GetLeftTopBlock();
};

namespace mastercore
{
	//! LevelDB based storage for the MetaDEx fee distributions
    extern COmniBlockHistory* p_blockhistory;
}

#endif // OMNICORE_BLOCK_HISTORY_H
