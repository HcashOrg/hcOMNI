#ifndef OMNICORE_BLOCK_HISTORY_H
#define OMNICORE_BLOCK_HISTORY_H

#include "omnicore/dbbase.h"
#include "omnicore/log.h"

#include <boost/filesystem.hpp>

#include <stdint.h>
#include <set>
#include <string>
#include <utility>

typedef std::pair<int, int64_t> feeCacheItem;
typedef std::pair<std::string, int64_t> feeHistoryItem;


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
    /** Count Fee History DB records */
    int CountRecords();
   
	//key euqal hash
	bool GetBlockHistory(const std::string& key, int& height, std::string& hash);

	bool GetBlockHistory(int index, int& height, std::string& hash);
	bool GetEndHistory(int& height, std::string& hash);
	int GetTopBlock();
	bool PutBlockHistory(int height, const std::string& hash);
};

namespace mastercore
{
	//! LevelDB based storage for the MetaDEx fee distributions
    extern COmniBlockHistory* p_blockhistory;
}

#endif // OMNICORE_BLOCK_HISTORY_H
