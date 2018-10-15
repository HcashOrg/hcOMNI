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
    void RollBackHistory(int block);
    /** Count Fee History DB records */
    int CountRecords();
    /** Record a fee distribution */
    void RecordFeeDistribution(const uint32_t &propertyId, int block, int64_t total, std::set<feeHistoryItem> feeRecipients);
    /** Retrieve the recipients for a fee distribution */
    std::set<feeHistoryItem> GetFeeDistribution(int id);
    /** Retrieve fee distributions for a property */
    std::set<int> GetDistributionsForProperty(const uint32_t &propertyId);
    /** Populate data about a fee distribution */
    bool GetDistributionData(int id, uint32_t *propertyId, int *block, int64_t *total);

	bool GetBlockHistory(int index, int& height, std::string& hash);
	bool GetEndHistory(int& height, std::string& hash);

	bool PutBlockHistory(int height, const std::string& hash);
};

namespace mastercore
{
	//! LevelDB based storage for the MetaDEx fee distributions
    extern COmniBlockHistory* p_blockhistory;
}

#endif // OMNICORE_BLOCK_HISTORY_H
