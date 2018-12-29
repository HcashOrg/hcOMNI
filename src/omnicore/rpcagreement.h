#ifndef OMNICORE_AGREEMENT_H
#define OMNICORE_AGREEMENT_H

#include <univalue.h>

UniValue omni_createagreement(const UniValue& params, bool fHelp);
UniValue omni_sendagreement(const UniValue& params, bool fHelp);
UniValue omni_acceptagreement(const UniValue& params, bool fHelp);
UniValue omni_cancelagreement(const UniValue& params, bool fHelp);

#endif // OMNICORE_AGREEMENT_H
