#include "omnicore/omniHelper.h"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>


#include <iostream>
#include <sstream>
#include <openssl/aes.h>


namespace OmniCore {

	std::string Compress(const std::string& input)
	{
		boost::iostreams::zlib_params param = {boost::iostreams::zlib::default_compression};
		std::stringstream ss_comp;
		std::stringstream ss_input;
		ss_input << input;
		boost::iostreams::filtering_ostream out;
		out.push(boost::iostreams::zlib_compressor(param));
		out.push(ss_comp);
		boost::iostreams::copy(ss_input, out);
		return ss_comp.str();
	}

	std::string Decompress(const std::string& input)
	{
		boost::iostreams::zlib_params param = {boost::iostreams::zlib::best_compression};
		std::stringstream ss_decomp;
		std::stringstream ss_input;
		ss_input << input;
		boost::iostreams::filtering_istream in;
		in.push(boost::iostreams::zlib_decompressor(param));
		in.push(ss_input);
		boost::iostreams::copy(in, ss_decomp);
		return ss_decomp.str();
	}

	std::string AesEncrypt(const std::string& input, const std::string& key)
	{
		unsigned char aes_keybuf[32];
		memset(aes_keybuf, 0, sizeof(aes_keybuf));
		memcpy((char *)aes_keybuf, key.c_str(), key.size() > 32 ? 32 : key.size());
		AES_KEY aeskey;
		AES_set_encrypt_key(aes_keybuf, 256, &aeskey);

		char *in_data = new char[17];
		char *out_data = new char[17];
		std::string in(input);
		std::string ret;
		while (!in.empty())
		{
			memset(in_data, 0, sizeof(char) * 17);
			memset(out_data, 0, sizeof(char) * 17);
			if (in.size() > 16)
			{
				memcpy(in_data, in.c_str(), 16);
				in = in.substr(16);
			}
			else
			{
				memcpy(in_data, in.c_str(), in.size());
				in.clear();
			}
			
			AES_encrypt((const unsigned char *)in_data, (unsigned char *)out_data, &aeskey);
			ret.append(out_data, 16);
		}
		delete[] in_data;
		delete[] out_data;
		return ret;
	}

	std::string AesDecrypt(const std::string& input, const std::string& key)
	{
		unsigned char aes_keybuf[32];
		memset(aes_keybuf, 0, sizeof(aes_keybuf));
		memcpy((char *)aes_keybuf, key.c_str(), key.size() > 32 ? 32 : key.size());
		AES_KEY aeskey;
		AES_set_decrypt_key(aes_keybuf, 256, &aeskey);

		char *in_data = new char[17];
		char *out_data = new char[17];
		std::string in(input);
		std::string ret;
		while (!in.empty())
		{
			memset(in_data, 0, sizeof(char) * 17);
			memset(out_data, 0, sizeof(char) * 17);
			if (in.size() > 16)
			{
				memcpy(in_data, in.c_str(), 16);
				in = in.substr(16);
			}
			else
			{
				memcpy(in_data, in.c_str(), in.size());
				in.clear();
			}
			AES_decrypt((const unsigned char *)in_data, (unsigned char *)out_data, &aeskey);
			ret.append(out_data, 16);
		}
		delete[] in_data;
		delete[] out_data;
		return ret;
	}

	std::string AesEncryptEnhance(const std::string& input, const std::string& key)
	{
		std::string newKey = Compress(key);
		if (newKey.size() < 32 )
		{
			newKey += key;
		}
		return Compress(AesEncrypt(AesEncrypt(input, newKey), key));
	}
	std::string AesDecryptEnhance(const std::string& input, const std::string& key)
	{
		std::string newKey = Compress(key);
		if (newKey.size() < 32)
		{
			newKey += key;
		}
		return AesDecrypt(AesDecrypt(Decompress(input), key), newKey);
	}
}