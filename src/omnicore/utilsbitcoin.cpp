/**
 * @file utilsbitcoin.cpp
 *
 * This file contains certain helpers to access information about Bitcoin.
 */

#include "chain.h"
#include "chainparams.h"
#include "main.h"
#include "sync.h"
#include "dbTxHistory.h"
#include "univalue.h"
#include "omnicore.h"

#include <stdint.h>
#include <string>

namespace mastercore
{
/**
 * @return The current chain length.
 */
int GetHeight()
{
	return _LatestBlock;
    //LOCK(cs_main);
    //return chainActive.Height();
	//if(!p_txhistory)
	//	return -1;
	//std::string history = p_txhistory->GetEndHistory();
	//int i=1;
	//int blockHeight = -1;
	//UniValue txobj;
	//if(txobj.read(history))
	//{
	//	blockHeight = txobj["Block"].get_int();
	//}
	//return blockHeight;
}

/**
 * @return The timestamp of the latest block.
 */
uint32_t GetLatestBlockTime()
{
    LOCK(cs_main);
    if (chainActive.Tip())
        return chainActive.Tip()->GetBlockTime();
    else
        return Params().GenesisBlock().nTime;
}

/**
 * @return The CBlockIndex, or NULL, if the block isn't available.
 */
CBlockIndex* GetBlockIndex(const uint256& hash)
{
    CBlockIndex* pBlockIndex = NULL;
    LOCK(cs_main);
    BlockMap::const_iterator it = mapBlockIndex.find(hash);
    if (it != mapBlockIndex.end()) {
        pBlockIndex = it->second;
    }

    return pBlockIndex;
}

bool MainNet()
{
    return Params().NetworkIDString() == "main";
}

bool TestNet()
{
    return Params().NetworkIDString() == "test";
}

bool RegTest()
{
    return Params().NetworkIDString() == "regtest";
}

bool UnitTest()
{
    return Params().NetworkIDString() == "unittest";
}

bool isNonMainNet()
{
    return !MainNet() && !UnitTest();
}


} // namespace mastercore
