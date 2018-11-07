#ifndef OMNICORE_TXHISTORY_H
#define OMNICORE_TXHISTORY_H

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
class COmniTxHistory : public CDBBase
{
public:
    COmniTxHistory(const boost::filesystem::path& path, bool fWipe);
    virtual ~COmniTxHistory();

    /** Show Fee History DB statistics */
    void printStats();
    /** Show Fee History DB records */
    void printAll();

    /** Roll back history in event of reorg */
    void RollBackHistory(int block);
    /** Count Fee History DB records */
    int64_t CountRecords()const;
   
	std::string GetHistory(const std::string& key);
	std::string GetHistory(int64_t index);
	std::string GetEndHistory();
	int64_t GetTop()const;
	bool IsExist(int64_t index)const;
	bool PutHistory(const std::string& Key, int nBlock, const std::string& history);
};

namespace mastercore
{
	//! LevelDB based storage for the MetaDEx fee distributions
    extern COmniTxHistory* p_txhistory;
}

#endif // OMNICORE_TXHISTORY_H
