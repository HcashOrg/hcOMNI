#ifndef OMNICORE_OMNICORE_H
#define OMNICORE_OMNICORE_H

class CBitcoinAddress;
class CBlockIndex;
class CCoinsView;
class CCoinsViewCache;
class CTransaction;

#include "omnicore/log.h"
#include "omnicore/tally.h"
#include "OmniThread/OmniThread.h"

#include "sync.h"
#include "uint256.h"
#include "util.h"

#include <univalue.h>

#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>

int const MAX_STATE_HISTORY = 50;
int const STORE_EVERY_N_BLOCK = 10000;

#define TEST_ECO_PROPERTY_1 (0x80000003UL)

// increment this value to force a refresh of the state (similar to --startclean)
#define DB_VERSION 6

// could probably also use: int64_t maxInt64 = std::numeric_limits<int64_t>::max();
// maximum numeric values from the spec:
#define MAX_INT_8_BYTES (9223372036854775807UL)

// maximum size of string fields
#define SP_STRING_FIELD_LEN 256

// Omni Layer Transaction (Packet) Version
#define MP_TX_PKT_V0  0
#define MP_TX_PKT_V1  1


// Transaction types, from the spec
enum TransactionType {
  MSC_TYPE_SIMPLE_SEND                =  0,
  MSC_TYPE_RESTRICTED_SEND            =  2,
  MSC_TYPE_SEND_TO_OWNERS             =  3,
  MSC_TYPE_SEND_ALL                   =  4,
  MSC_TYPE_SAVINGS_MARK               = 10,
  MSC_TYPE_SAVINGS_COMPROMISED        = 11,
  MSC_TYPE_RATELIMITED_MARK           = 12,
  MSC_TYPE_AUTOMATIC_DISPENSARY       = 15,
  MSC_TYPE_TRADE_OFFER                = 20,
  MSC_TYPE_ACCEPT_OFFER_BTC           = 22,
  MSC_TYPE_METADEX_TRADE              = 25,
  MSC_TYPE_METADEX_CANCEL_PRICE       = 26,
  MSC_TYPE_METADEX_CANCEL_PAIR        = 27,
  MSC_TYPE_METADEX_CANCEL_ECOSYSTEM   = 28,
  MSC_TYPE_NOTIFICATION               = 31,
  MSC_TYPE_OFFER_ACCEPT_A_BET         = 40,
  MSC_TYPE_CREATE_PROPERTY_FIXED      = 50,
  MSC_TYPE_CREATE_PROPERTY_VARIABLE   = 51,
  MSC_TYPE_PROMOTE_PROPERTY           = 52,
  MSC_TYPE_CLOSE_CROWDSALE            = 53,
  MSC_TYPE_CREATE_PROPERTY_MANUAL     = 54,
  MSC_TYPE_GRANT_PROPERTY_TOKENS      = 55,
  MSC_TYPE_REVOKE_PROPERTY_TOKENS     = 56,
  MSC_TYPE_CHANGE_ISSUER_ADDRESS      = 70,
  MSC_TYPE_ENABLE_FREEZING            = 71,
  MSC_TYPE_DISABLE_FREEZING           = 72,
  MSC_TYPE_FREEZE_PROPERTY_TOKENS     = 185,
  MSC_TYPE_UNFREEZE_PROPERTY_TOKENS   = 186,
  MSC_TYPE_SEND_AGREEMENT             = 202,
  OMNICORE_MESSAGE_TYPE_DEACTIVATION  = 65533,
  OMNICORE_MESSAGE_TYPE_ACTIVATION    = 65534,
  OMNICORE_MESSAGE_TYPE_ALERT         = 65535
};

#define MSC_PROPERTY_TYPE_INDIVISIBLE             1
#define MSC_PROPERTY_TYPE_DIVISIBLE               2
#define MSC_PROPERTY_TYPE_INDIVISIBLE_REPLACING   65
#define MSC_PROPERTY_TYPE_DIVISIBLE_REPLACING     66
#define MSC_PROPERTY_TYPE_INDIVISIBLE_APPENDING   129
#define MSC_PROPERTY_TYPE_DIVISIBLE_APPENDING     130

#define PKT_RETURNED_OBJECT    (1000)

#define PKT_ERROR             ( -9000)
#define DEX_ERROR_SELLOFFER   (-10000)
#define DEX_ERROR_ACCEPT      (-20000)
#define DEX_ERROR_PAYMENT     (-30000)
// Smart Properties
#define PKT_ERROR_SP          (-40000)
#define PKT_ERROR_CROWD       (-45000)
// Send To Owners
#define PKT_ERROR_STO         (-50000)
#define PKT_ERROR_SEND        (-60000)
#define PKT_ERROR_TRADEOFFER  (-70000)
#define PKT_ERROR_METADEX     (-80000)
#define METADEX_ERROR         (-81000)
#define PKT_ERROR_TOKENS      (-82000)
#define PKT_ERROR_SEND_ALL    (-83000)

#define OMNI_PROPERTY_BTC   0
#define OMNI_PROPERTY_MSC   1
#define OMNI_PROPERTY_TMSC  2

/** Number formating related functions. */
std::string FormatDivisibleMP(int64_t amount, bool fSign = false);
std::string FormatDivisibleShortMP(int64_t amount);
std::string FormatIndivisibleMP(int64_t amount);
std::string FormatByType(int64_t amount, uint16_t propertyType);
// Note: require initialized state to get divisibility.
std::string FormatMP(uint32_t propertyId, int64_t amount, bool fSign = false);
std::string FormatShortMP(uint32_t propertyId, int64_t amount);

/** Returns the Exodus address. */
const std::string& ExodusAddress();

/** Returns the Exodus crowdsale address. */
const CBitcoinAddress ExodusCrowdsaleAddress(int nBlock = 0);

/** Returns the marker for class C transactions. */
const std::vector<unsigned char> GetOmMarker();

//! Used to indicate, whether to automatically commit created transactions
extern bool autoCommit;

//! Global lock for state objects
extern CCriticalSection cs_tally;

//! Available balances of wallet properties
extern std::map<uint32_t, int64_t> global_balance_money;
//! Reserved balances of wallet propertiess
extern std::map<uint32_t, int64_t> global_balance_reserved;
//! Vector containing a list of properties relative to the wallet
extern std::set<uint32_t> global_wallet_property_list;

int64_t GetTokenBalance(const std::string& address, uint32_t propertyId, TallyType ttype);
int64_t GetAvailableTokenBalance(const std::string& address, uint32_t propertyId);
int64_t GetReservedTokenBalance(const std::string& address, uint32_t propertyId);
int64_t GetFrozenTokenBalance(const std::string& address, uint32_t propertyId);

/** Global handler to initialize Omni Core. */
int mastercore_init();
int mastercore_init_ex();

extern CDllDataHandler g_data_handler;

/** Global handler to shut down Omni Core. */
int mastercore_shutdown();

void RewindDBs(int nHeight, int top, bool fInitialParse = false);
void RewindDBsAndState(int nHeight, int nBlockPrev = 0, bool fInitialParse = false);

/** Block and transaction handlers. */
int mastercore_handler_disc_begin(int nBlockNow, CBlockIndex const * pBlockIndex);
int mastercore_handler_disc_end(int nBlockNow, CBlockIndex const * pBlockIndex);
int mastercore_handler_block_begin(int nBlockNow, CBlockIndex const * pBlockIndex);
int mastercore_handler_block_end(int nBlockNow, CBlockIndex const * pBlockIndex, unsigned int);
bool mastercore_handler_tx(const CTransaction& tx, int nBlock, unsigned int idx, const CBlockIndex* pBlockIndex);
bool mastercore_handler_mptx(const UniValue &root);
void clear_all_state();

void SetWaterline(int water_line);
int GetWaterlineBlock();

bool TXExodusFundraiser(const std::string& hash, const std::string& sender, int64_t amountInvested, int nBlock, unsigned int nTime);

/** Global handler to total wallet balances. */
void CheckWalletUpdate(bool forceUpdate = false);

/** Used to notify that the number of tokens for a property has changed. */
void NotifyTotalTokensChanged(uint32_t propertyId, int block);

std::string PayLoadWrap(const std::vector<unsigned char>& payload);

namespace mastercore
{
//! In-memory collection of all amounts for all addresses for all properties
extern std::unordered_map<std::string, CMPTally> mp_tally_map;

// TODO: move, rename
extern CCoinsView viewDummy;
extern CCoinsViewCache view;
//! Guards coins view cache
extern CCriticalSection cs_tx_cache;

extern int _LatestBlock;
extern uint256 _LatestBlockHash;
extern int64_t _LatestBlockTime;

/** Returns the encoding class, used to embed a payload. */
int GetEncodingClass(const CTransaction& tx, int nBlock);

/** Determines, whether it is valid to use a Class C transaction for a given payload size. */
bool UseEncodingClassC(size_t nDataSize);

bool isTestEcosystemProperty(uint32_t propertyId);
bool isMainEcosystemProperty(uint32_t propertyId);
uint32_t GetNextPropertyId(bool maineco); // maybe move into sp

CMPTally* getTally(const std::string& address);
bool update_tally_map(const std::string& who, uint32_t propertyId, int64_t amount, TallyType ttype);
int64_t getTotalTokens(uint32_t propertyId, int64_t* n_owners_total = NULL);

std::string strMPProperty(uint32_t propertyId);
std::string strTransactionType(uint16_t txType);
std::string getTokenLabel(uint32_t propertyId);

/**
    NOTE: The following functions are only permitted for properties
          managed by a central issuer that have enabled freezing.
 **/
/** Adds an address and property to the frozenMap **/
void freezeAddress(const std::string& address, uint32_t propertyId);
/** Removes an address and property from the frozenMap **/
void unfreezeAddress(const std::string& address, uint32_t propertyId);
/** Checks whether an address and property are frozen **/
bool isAddressFrozen(const std::string& address, uint32_t propertyId);
/** Adds a property to the freezingEnabledMap **/
void enableFreezing(uint32_t propertyId, int liveBlock);
/** Removes a property from the freezingEnabledMap **/
void disableFreezing(uint32_t propertyId);
/** Checks whether a property has freezing enabled **/
bool isFreezingEnabled(uint32_t propertyId, int block);
/** Clears the freeze state in the event of a reorg **/
void ClearFreezeState();
/** Prints the freeze state **/
void PrintFreezeState();

}

#endif // OMNICORE_OMNICORE_H
