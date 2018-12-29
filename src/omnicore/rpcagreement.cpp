#include "omnicore/rpcagreement.h"

#include "omnicore/createpayload.h"
#include "omnicore/rpcvalues.h"
#include "omnicore/rpcrequirements.h"
#include "omnicore/omnicore.h"
#include "omnicore/sp.h"
#include "omnicore/tx.h"

#include "rpc/server.h"
#include "utilstrencodings.h"

#include <univalue.h>

using std::runtime_error;
using namespace mastercore;

UniValue omni_createagreement(const UniValue& params, bool fHelp)
{
	if (fHelp || params.size() < 2 || params.size() >4)
		throw runtime_error(
			"omni_createagreement propertyid \"amount\"\n"

			"\nCreate the agreement.\n"

			"\nArguments:\n"
			"1. agreementid     (string, required) the content to send\n"
			"2. nonencryptedcontent	 (string, required) the Non encrypted content\n"
			"3. key					 (string, required) the key of the encrypted content\n"
			"4. encryptedcontent     (string, required) the encrypted content\n"

			"\nResult:\n"
			"\"payload\"             (string) the hex-encoded payload\n"

			"\nExamples:\n"
			+ HelpExampleCli("omni_createagreement", "agreementid \"nonencryptedcontent\" \"key\" \"encryptedcontent\" ")
			+ HelpExampleRpc("omni_createagreement", "agreementid \"nonencryptedcontent\" \"key\" \"encryptedcontent\" ")
		);

	std::string agreementid = params[0].get_str();
	std::string nonencryptedcontent = params[2].get_str();

	std::string key;
	std::string encryptedcontent;
	if (params.size() == 4) {
		key = params[3].get_str();
		encryptedcontent = params[5].get_str();
	}

	std::vector<unsigned char> payload = CreatePayload_OmniAgreement(agreementid, nonencryptedcontent, key, encryptedcontent);
	return HexStr(payload.begin(), payload.end());
}

UniValue omni_sendagreement(const UniValue& params, bool fHelp)
{
	if (fHelp || params.size() < 4 || params.size() > 6)
		throw runtime_error(
			"omni_createagreement propertyid \"amount\"\n"

			"\nCreate the agreement.\n"

			"\nArguments:\n"
			"1. fromaddress          (string, required) the address to send from\n"
			"2. toaddress            (string, required) the address of the receiver\n"
			"3. agreementid          (string, required) the type of the agreement\n"
			"4. nonencryptedcontent	 (string, required) the Non encrypted content\n"
			"5. key					 (string, required) the key of the encrypted content\n"
			"6. encryptedcontent     (string, required) the encrypted content\n"

			"\nResult:\n"
			"\"payload\"             (string) the hex-encoded payload\n"

			"\nExamples:\n"
			+ HelpExampleCli("omni_createagreement", "fromaddress" "toaddress" "agreementid \"nonencryptedcontent\" \"key\" \"encryptedcontent\" ")
			+ HelpExampleRpc("omni_createagreement", "fromaddress" "toaddress" "agreementid \"nonencryptedcontent\" \"key\" \"encryptedcontent\" ")
		);

	std::string fromAddress = params[0].getValStr(); //ParseAddress(params[0]);
	std::string toAddress = params[1].getValStr();   //ParseAddress(params[1]);

	std::string agreementid = params[2].getValStr();
	std::string nonencryptedcontent = params[3].getValStr();

	std::string key;
	std::string encryptedcontent;
	if (params.size() == 6) {
		key = params[4].getValStr();
		encryptedcontent = params[5].getValStr();
	}

	std::vector<unsigned char> payload = CreatePayload_OmniAgreement(agreementid, nonencryptedcontent, key, encryptedcontent);
	return PayLoadWrap(payload);
}

UniValue omni_acceptagreement(const UniValue& params, bool fHelp)
{
	if (fHelp || params.size() < 4 || params.size() > 6)
		throw runtime_error(
			"omni_createagreement propertyid \"amount\"\n"

			"\nCreate the agreement.\n"

			"\nArguments:\n"
			"1. fromaddress          (string, required) the address to send from\n"
			"2. toaddress            (string, required) the address of the receiver\n"
			"3. agreementid          (string, required) the type of the agreement\n"
			"4. nonencryptedcontent	 (string, required) the Non encrypted content\n"
			"5. key					 (string, required) the key of the encrypted content\n"
			"6. encryptedcontent     (string, required) the encrypted content\n"

			"\nResult:\n"
			"\"payload\"             (string) the hex-encoded payload\n"

			"\nExamples:\n"
			+ HelpExampleCli("omni_createagreement", "fromaddress" "toaddress" "agreementid \"nonencryptedcontent\" \"key\" \"encryptedcontent\" ")
			+ HelpExampleRpc("omni_createagreement", "fromaddress" "toaddress" "agreementid \"nonencryptedcontent\" \"key\" \"encryptedcontent\" ")
		);

	std::string fromAddress = params[0].getValStr(); //ParseAddress(params[0]);
	std::string toAddress = params[1].getValStr();   //ParseAddress(params[1]);

	std::string agreementid = params[2].getValStr();
	std::string nonencryptedcontent = params[3].getValStr();

	std::string key;
	std::string encryptedcontent;
	if (params.size() == 6) {
		key = params[4].getValStr();
		encryptedcontent = params[5].getValStr();
	}

	std::vector<unsigned char> payload = CreatePayload_OmniAgreement(agreementid, nonencryptedcontent, key, encryptedcontent);
	return PayLoadWrap(payload);
}

UniValue omni_cancelagreement(const UniValue& params, bool fHelp)
{
	if (fHelp || params.size() != 2)
		throw runtime_error(
			"omni_cancelagreement propertyid \"amount\"\n"

			"\nCreate the agreement.\n"

			"\nArguments:\n"
			"1. agreementtype        (string, required) the type of the agreement\n"
			"2. agreementcontent     (string, required) the content to send\n"

			"\nResult:\n"
			"\"payload\"             (string) the hex-encoded payload\n"

			"\nExamples:\n"
			+ HelpExampleCli("omni_cancelagreement", "aaaaaaaaaaa \"bbbbbbbbbbb\"")
			+ HelpExampleRpc("omni_cancelagreement", "aaaaaaaaaaa, \"bbbbbbbbbbb\"")
		);

	std::string type = params[0].get_str();
	std::string content = params[1].get_str();

	std::vector<unsigned char> payload = CreatePayload_OmniAgreement(type, "", "", "");
	return HexStr(payload.begin(), payload.end());
}

static const CRPCCommand commands[] =
{ //  category                         name                                      actor (function)                         okSafeMode
  //  -------------------------------- ----------------------------------------- ---------------------------------------- ----------
	{ "omni layer (payload creation)", "omni_createagreement",					 &omni_createagreement,					  true },
	{ "omni layer (payload creation)", "omni_sendagreement",					 &omni_sendagreement,					  true },
	{ "omni layer (payload creation)", "omni_acceptagreement",					 &omni_acceptagreement,					  true },
	{ "omni layer (payload creation)", "omni_cancelagreement",					 &omni_cancelagreement,					  true },
};

void RegisterOmniAgreementRPCCommands(CRPCTable &tableRPC)
{
	for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
		tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
