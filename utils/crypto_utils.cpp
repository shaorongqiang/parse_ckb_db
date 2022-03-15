#include <arpa/inet.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/oids.h>
#include <cryptopp/osrng.h>
#include <cryptopp/ripemd.h>
#include <libbase58.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "utils/crypto_utils.h"
#include <cryptopp/adler32.h>
#include <cryptopp/base64.h>
#include <cryptopp/md5.h>
#include <cryptopp/zlib.h>
#include <zlib.h>

std::string Bytes2Hex(const std::string &bytes,  bool to_uppercase)
{
    std::string hex;
    CryptoPP::HexEncoder hex_encoder(nullptr, to_uppercase);
    hex_encoder.Attach(new CryptoPP::StringSink(hex));
    hex_encoder.Put((const CryptoPP::byte *)bytes.data(), bytes.length());
    hex_encoder.MessageEnd();
    return hex;
}

std::string Hex2Bytes(const std::string &hex)
{
    std::string bytes;
    CryptoPP::HexDecoder hex_decoder;
    hex_decoder.Attach(new CryptoPP::StringSink(bytes));
    hex_decoder.Put((const CryptoPP::byte *)hex.data(), hex.length());
    hex_decoder.MessageEnd();
    return bytes;
}

std::string Base58Encode(const std::string &bytes)
{
    std::string base58;
    size_t b58sz = bytes.length();
    b58sz = b58sz * 138 / 100 + 1;
    char * b58 = new char[b58sz]{0};
    bool flag = b58enc(b58, &b58sz, bytes.data(), bytes.length());
    if (flag)
    {
        base58 = std::move(std::string(b58, strlen(b58)));
    }
    delete [] b58;
    return base58;
}

std::string Base58Decode(const std::string &base58)
{
    std::string bytes;
    size_t binsz = base58.length();
    char * bin = new char[binsz] {0};
    bool flag = b58tobin(bin, &binsz, base58.data(), base58.length());
    if (flag)
    {
        bytes = std::move(std::string(bin + sizeof(bin) - binsz, binsz));
    }
    delete [] bin;
    return bytes;
}

std::string Base64Encode(const std::string &bytes)
{
    std::string base64;
    CryptoPP::Base64Encoder base64_encode(nullptr, false);
    base64_encode.Attach(new CryptoPP::StringSink(base64));
    base64_encode.Put((const CryptoPP::byte *)bytes.data(), bytes.length());
    base64_encode.MessageEnd();
    return base64;
}

std::string Base64Decode(const std::string &base64)
{
    std::string bytes;
    CryptoPP::Base64Decoder base64_decoder;
    base64_decoder.Attach(new CryptoPP::StringSink(bytes));
    base64_decoder.Put((const CryptoPP::byte *)base64.data(), base64.length());
    base64_decoder.MessageEnd();
    return bytes;
}


std::string GetMd5Hash(const std::string &bytes, bool to_uppercase)
{
    std::string hash;
    CryptoPP::Weak::MD5 md5;
    CryptoPP::HashFilter hashfilter(md5);
    hashfilter.Attach(new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), to_uppercase));
    hashfilter.Put(reinterpret_cast<const uint8_t *>(bytes.c_str()), bytes.length());
    hashfilter.MessageEnd();
    return hash;
}

std::string GetSha1Hash(const std::string &bytes, bool to_uppercase)
{
    std::string hash;
    CryptoPP::SHA1 sha1;
    CryptoPP::HashFilter hashfilter(sha1);
    hashfilter.Attach(new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), to_uppercase));
    hashfilter.Put(reinterpret_cast<const uint8_t *>(bytes.c_str()), bytes.length());
    hashfilter.MessageEnd();
    return hash;
}

std::string GetSha256Hash(const std::string &bytes, bool to_uppercase)
{
    std::string hash;
    CryptoPP::SHA256 sha256;
    CryptoPP::HashFilter hashfilter(sha256);
    hashfilter.Attach(new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), to_uppercase));
    hashfilter.Put(reinterpret_cast<const uint8_t *>(bytes.c_str()), bytes.length());
    hashfilter.MessageEnd();
    return hash;
}

std::string GetRipemd160Hash(const std::string &bytes, bool to_uppercase)
{
    std::string hash;
    CryptoPP::RIPEMD160 ripemd160;
    CryptoPP::HashFilter hashfilter(ripemd160);
    hashfilter.Attach(new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), to_uppercase));
    hashfilter.Put(reinterpret_cast<const uint8_t *>(bytes.c_str()), bytes.length());
    hashfilter.MessageEnd();
    return hash;
}
