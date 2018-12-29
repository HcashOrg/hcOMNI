#ifndef OMNICORE_DBAGREEMENTTRADELIST_H
#define OMNICORE_DBAGREEMENTTRADELIST_H

#include "omnicore/dbbase.h"

#include "uint256.h"

#include <univalue.h>

#include <boost/filesystem/path.hpp>

#include <stdint.h>

#include <string>
#include <vector>

/** LevelDB based storage for the agreement trade history. Trades are listed with key "txid1+txid2".
 */
class CMPAgtTradeList : public CDBBase
{
public:
    CMPAgtTradeList(const boost::filesystem::path& path, bool fWipe);
    virtual ~CMPAgtTradeList();

	//record new trade
	void recordNewTrade(const uint256& txid, const std::string& sender,
		const std::string& receiver, const std::string& agt_id, int blockNum, int blockIndex);

	// Roll back history in event of reorg, block is inclusive
	void RollBackHistory(int64_t block);

};

namespace mastercore
{
    //! LevelDB based storage for the agreement trade history
    extern CMPAgtTradeList* t_agttradelistdb;
}

#endif // OMNICORE_DBAGREEMENTTRADELIST_H
