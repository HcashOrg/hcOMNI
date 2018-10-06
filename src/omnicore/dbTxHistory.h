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
    int CountRecords();
    /** Record a fee distribution */
    void RecordFeeDistribution(const uint32_t &propertyId, int block, int64_t total, std::set<feeHistoryItem> feeRecipients);
    /** Retrieve the recipients for a fee distribution */
    std::set<feeHistoryItem> GetFeeDistribution(int id);
    /** Retrieve fee distributions for a property */
    std::set<int> GetDistributionsForProperty(const uint32_t &propertyId);
    /** Populate data about a fee distribution */
    bool GetDistributionData(int id, uint32_t *propertyId, int *block, int64_t *total);

	std::string GetHistory(int index);
	std::string GetEndHistory();

	bool PutHistory(const std::string& history);
};

namespace mastercore
{
	//! LevelDB based storage for the MetaDEx fee distributions
    extern COmniTxHistory* p_txhistory;
}

#endif // OMNICORE_TXHISTORY_H
