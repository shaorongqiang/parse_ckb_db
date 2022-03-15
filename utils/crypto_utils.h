#ifndef _UTILS_CRYPTO_UTILS_H__
#define _UTILS_CRYPTO_UTILS_H__

#include <cryptopp/eccrypto.h>
#include <cryptopp/sha.h>
#include <string>

std::string Bytes2Hex(const std::string &bytes, bool to_uppercase = false);
std::string Hex2Bytes(const std::string &hex);

std::string Base58Encode(const std::string &bytes);
std::string Base58Decode(const std::string &base58);

std::string Base64Encode(const std::string &bytes);
std::string Base64Decode(const std::string &base64);

std::string GetMd5Hash(const std::string &bytes, bool to_uppercase = false);
std::string GetSha1Hash(const std::string &bytes, bool to_uppercase = false);
std::string GetSha256Hash(const std::string &bytes, bool to_uppercase = false);
std::string GetRipemd160Hash(const std::string &bytes, bool to_uppercase = false);

#endif
