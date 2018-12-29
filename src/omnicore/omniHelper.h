#ifndef OMNICORE_OMNIHELPER_H
#define OMNICORE_OMNIHELPER_H

#include <string>
namespace OmniCore{

	std::string Compress(const std::string& input);
	std::string Decompress(const std::string& input);
	std::string AesEncrypt(const std::string& input, const std::string& key);
	std::string AesDecrypt(const std::string& input, const std::string& key);

	std::string AesEncryptEnhance(const std::string& input, const std::string& key);
	std::string AesDecryptEnhance(const std::string& input, const std::string& key);

}

#endif // OMNICORE_AGREEMENT_H
