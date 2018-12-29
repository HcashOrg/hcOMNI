#include "omnicore/dbagreementtradelist.h"
#include "omnicore/log.h"


CMPAgtTradeList::CMPAgtTradeList(const boost::filesystem::path& path, bool fWipe)
{
    leveldb::Status status = Open(path, fWipe);
    PrintToConsole("Loading AgtTrade database: %s\n", status.ToString());
}

CMPAgtTradeList::~CMPAgtTradeList()
{
    if (msc_debug_persistence)
        PrintToLog("CMPTradeList closed\n");
}

void CMPAgtTradeList::recordNewTrade(const uint256& txid, const std::string& sender,
	const std::string& receiver, const std::string& agt_id, int blockNum, int blockIndex) 
{
	if (!pdb)
		return;
	std::string strValue = strprintf("%s:%s:%s:%d:%d", sender, receiver, agt_id, blockNum, blockIndex);
	leveldb::Status status = pdb->Put(writeoptions, txid.ToString(), strValue);
	++nWritten;
	if (msc_debug_tradedb)
		PrintToLog("%s: %s\n", __func__, status.ToString());
}

// Roll back history in event of reorg, block is inclusive
void CMPAgtTradeList::RollBackHistory(int64_t block)
{
	assert(pdb);
	int count = 0;
	leveldb::Iterator* it = NewIterator();
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		++count;
		int64_t key = 0;//atoi64(it->key().ToString().c_str());
		if (key >= block)
		{
			pdb->Delete(writeoptions, it->key());
		}
	}
	delete it;
}