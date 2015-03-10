//
//  Encoder.cpp
//  receiptdaemon
//
//  Created by Zsolt Szatmari on 01/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <functional>
#include <iostream>
#include <sstream>
#include "openssl/rand.h"
#include "openssl/rsa.h"
#include "openssl/engine.h"
#include "Encoder.h"
#include "base64.h"
#include "stdplus.h"
#include MEMORY_H

#ifdef __APPLE__
namespace Recd
{
#define method Encoder::
    
    using std::string;
    using std::vector;
    //using std::unique_ptr;
    //using std::function;
    
#ifdef __APPLE__
#define deleter_ptr MEMORY_NS::shared_ptr
#else
#define deleter_ptr MEMORY_NS::shared_ptr
//#error please define deleter_ptr as unique_ptr<T,function<void(T *)>>
#endif
    
    // seal&open: encrypt with public key, decrypt with private key
    // sign&verify: encrypt with private key, decrypt with public key
    
    string method base64encode(const vector<unsigned char> &data)
    {
        size_t length;
        char *str = base64_encode(data.data(), data.size(), &length);
        string ret(str, length);
        free(str);
        return ret;
    }
    
    vector<unsigned char> method base64decode(const string &str)
    {
        size_t length;
        unsigned char *data = base64_decode(str.data(), str.size(), &length);
        if (data == NULL) {
            // invalid
            return vector<unsigned char>();
        }
        vector<unsigned char> ret(data, data+length);
    
        free(data);
        return ret;
    }
    
    static deleter_ptr<EVP_PKEY> parsePublicKey(const vector<unsigned char> &publicKey)
    {
        const unsigned char *tmp = publicKey.data();
        auto deleter = [](EVP_PKEY *pkey){
            EVP_PKEY_free(pkey);
        };
        return deleter_ptr<EVP_PKEY>(d2i_PUBKEY(nullptr, &tmp, publicKey.size()), deleter);
    }
    
    static deleter_ptr<EVP_PKEY> parsePrivateKey(const vector<unsigned char> &privateKey)
    {
        const unsigned char *tmp = privateKey.data();
        auto deleter = [](EVP_PKEY *pkey){
            EVP_PKEY_free(pkey);
        };
        
        return deleter_ptr<EVP_PKEY>(d2i_PrivateKey(EVP_PKEY_RSA, NULL, &tmp, privateKey.size()), deleter);
    }
    
    static deleter_ptr<EVP_PKEY_CTX> createKeyContext()
    {
        auto ctxDeleter = [](EVP_PKEY_CTX *ctx){
            EVP_PKEY_CTX_free(ctx);
        };
        deleter_ptr<EVP_PKEY_CTX> rsaCtx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL), ctxDeleter);
        
        return rsaCtx;
    }
    
    std::pair<vector<unsigned char>, vector<unsigned char>> method generatePublicPrivateKeys(int bits)
    {
        EVP_PKEY *keys = nullptr;
        auto ctx = createKeyContext();
        
        static std::pair<vector<unsigned char>, vector<unsigned char>> ret;
        if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
            return ret;
        }
        
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), bits) <= 0) {
            return ret;
        }
        
        if (EVP_PKEY_keygen(ctx.get(), &keys) <= 0) {
            return ret;
        }
        
        unsigned char *pub = nullptr;
        int len = i2d_PUBKEY(keys, &pub);
        ret.first = vector<unsigned char>(pub, pub + len);
        unsigned char *priv = nullptr;
        len = i2d_PrivateKey(keys, &priv);
        ret.second = vector<unsigned char>(priv, priv + len);
        
        EVP_PKEY_free(keys);
        
        return ret;
    }
    
    void method generateKeys(int bits)
    {
        auto keys = generatePublicPrivateKeys(bits);
    
        std::cout << "public key: " << base64encode(keys.first) << std::endl;
        std::cout << "private key: " << base64encode(keys.second) << std::endl;
    }

    static std::string sourceKey(const std::vector<unsigned char> &b)
    {
        std::stringstream ss;
        for (auto c : b) {
            ss << "0x" << std::hex << (int)c << ",";
        }
        return ss.str();
    }

    void method generateBinaryKeys(int bits, int offset)
    {
        auto keys = generatePublicPrivateKeys(bits);
        for (auto &c : keys.first) {
            c = c + offset;
        }
        for (auto &c : keys.second) {
            c = c + offset;
        }

        std::cout << "public key: " << sourceKey(keys.first) << std::endl;
        std::cout << "private key: " << sourceKey(keys.second) << std::endl;
    }
    
    static deleter_ptr<EVP_CIPHER_CTX> createContext()
    {
        auto ctxDeleter = [](EVP_CIPHER_CTX *ctx){
            EVP_CIPHER_CTX_cleanup(ctx);
            free(ctx);
        };
        deleter_ptr<EVP_CIPHER_CTX> rsaCtx((EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX)), ctxDeleter);
        EVP_CIPHER_CTX_init(rsaCtx.get());
        
        return rsaCtx;
    }
    
    static deleter_ptr<EVP_MD_CTX> createMdContext()
    {
        auto ctxDeleter = [](EVP_MD_CTX *ctx){
            EVP_MD_CTX_cleanup(ctx);
            free(ctx);
        };
        deleter_ptr<EVP_MD_CTX> rsaCtx((EVP_MD_CTX*)malloc(sizeof(EVP_MD_CTX)), ctxDeleter);
        EVP_MD_CTX_init(rsaCtx.get());
        
        return rsaCtx;
    }
    
    string method seal(const string &msg, const vector<unsigned char> &publicKey)
    {
        int encMsgLen = 0;
        int blockLen  = 0;
        
        //EVP_PKEY *d2i_PrivateKey(int type, EVP_PKEY **a, unsigned char **pp, long length);
        
        auto remotePubKey = parsePublicKey(publicKey);
        
        // ek: encrypted key
        vector<unsigned char> ek(EVP_PKEY_size(remotePubKey.get()));
        vector<unsigned char> iv(EVP_MAX_IV_LENGTH);
        
        vector<char> padded((msg.size() / 16 + 1) * 16, '\0');
        copy(msg.begin(), msg.end(), padded.begin());
        
        vector<unsigned char> encMsg(padded.size() + EVP_MAX_IV_LENGTH);
        
        auto rsaCtx = createContext();
        
        int ekl = 0;
        unsigned char *tmp2 = ek.data();
        auto tmp3 = remotePubKey.get();
        if(!EVP_SealInit(rsaCtx.get(), EVP_aes_128_cbc(), &tmp2, &ekl, iv.data(), &tmp3, 1)) {
            return "";
        }
        ek.resize(ekl);
                            
        // include terminating zero (because padding is random character)
        if(!EVP_SealUpdate(rsaCtx.get(), encMsg.data() + encMsgLen, &blockLen, reinterpret_cast<const unsigned char *>(padded.data()), (int)padded.size())) {
            return "";
        }
        encMsgLen += blockLen;
        
        if(!EVP_SealFinal(rsaCtx.get(), encMsg.data() + encMsgLen, &blockLen)) {
            return "";
        }
        encMsgLen += blockLen;
        
        vector<unsigned char> output;
        uint32_t ekSize = htonl(ek.size());
        uint32_t ivSize = htonl(iv.size());
        unsigned char *ekSizeP = reinterpret_cast<unsigned char *>(&ekSize);
        unsigned char *ivSizeP = reinterpret_cast<unsigned char *>(&ivSize);
        
        output.push_back(0);
        output.insert(output.end(), ekSizeP, ekSizeP + sizeof(uint32_t));
        output.insert(output.end(), ivSizeP, ivSizeP + sizeof(uint32_t));
        output.insert(output.end(), ek.begin(), ek.end());
        output.insert(output.end(), iv.begin(), iv.end());
        output.insert(output.end(), encMsg.begin(), encMsg.end());
        
        return base64encode(output);
    }
    
    string method open(const string &encodedBase, const vector<unsigned char> &privateKey)
    {
        vector<unsigned char> encoded = base64decode(encodedBase);
        if (encoded.size() < (1+2*sizeof(uint32_t))) {
            return "";
        }
        
        if (encoded.at(0) != 0) {
            return "";
        }
        uint32_t *ekSizeP = reinterpret_cast<uint32_t *>(&encoded.at(1));
        uint32_t *ivSizeP = reinterpret_cast<uint32_t *>(&encoded.at(1 + sizeof(uint32_t)));
        uint32_t ekSize = ntohl(*ekSizeP);
        uint32_t ivSize = ntohl(*ivSizeP);
        
        if (encoded.size() < (1+2*sizeof(uint32_t)+ekSize+ivSize)) {
            return "";
        }
        vector<unsigned char> ek(encoded.begin() + (1 + 2*sizeof(uint32_t)), encoded.begin() + (1 + 2*sizeof(uint32_t)) + ekSize);
        vector<unsigned char> iv(encoded.begin() + (1 + 2*sizeof(uint32_t)) + ekSize, encoded.begin() + (1 + 2*sizeof(uint32_t)) + ekSize + ivSize);
        vector<unsigned char> encMsg(encoded.begin() + (1 + 2*sizeof(uint32_t)) + ekSize + ivSize, encoded.end());
        
        size_t decLen = 0;
        int blockLen = 0;
        
        auto key = parsePrivateKey(privateKey);
        
        
        vector<unsigned char> decMsg(encMsg.size() + iv.size());
        
        auto rsaCtx = createContext();
        
        if(!EVP_OpenInit(rsaCtx.get(), EVP_aes_128_cbc(), ek.data(), (int)ek.size(), iv.data(), key.get())) {
            return "";
        }
        
        if(!EVP_OpenUpdate(rsaCtx.get(), decMsg.data() + decLen, &blockLen, encMsg.data(), (int)encMsg.size())) {
            return "";
        }
        decLen += blockLen;
    
        if(!EVP_OpenFinal(rsaCtx.get(), decMsg.data() + decLen, &blockLen)) {
            return "";
        }
        decLen += blockLen;
        
        size_t len = strnlen((const char *)decMsg.data(), decMsg.end() - decMsg.begin());
        return string(decMsg.begin(), decMsg.begin() + len);
    }
    
    string method sign(const string &msg, const vector<unsigned char> &privateKey)
    {
        auto privKey = parsePrivateKey(privateKey);
        
        //vector<char> padded((msg.size() / 16 + 1) * 16, '\0');
        //copy(msg.begin(), msg.end(), padded.begin());
        
        auto rsaCtx = createMdContext();
        
        if(!EVP_SignInit(rsaCtx.get(), EVP_sha256())) {
            return "";
        }
        
        if(!EVP_SignUpdate(rsaCtx.get(), msg.data(), msg.length())) {
            return "";
        }
        
        vector<unsigned char> sig(EVP_PKEY_size(privKey.get()));
        unsigned int sigLen = 0;
        if(!EVP_SignFinal(rsaCtx.get(), sig.data(), &sigLen, privKey.get())) {
            return "";
        }
        sig.resize(sigLen);
        
        return base64encode(sig);
    }
    
    bool method verifySignature(const std::string &msg, const std::vector<unsigned char> &publicKey, const std::string &signature)
    {
        auto pubKey = parsePublicKey(publicKey);
        
        //vector<char> padded((msg.size() / 16 + 1) * 16, '\0');
        //copy(msg.begin(), msg.end(), padded.begin());
        
        auto rsaCtx = createMdContext();
        
        if(!EVP_VerifyInit(rsaCtx.get(), EVP_sha256())) {
            return false;
        }
        
        if(!EVP_VerifyUpdate(rsaCtx.get(), msg.data(), msg.length())) {
            return false;
        }
        
        auto sig = base64decode(signature);
        if(!EVP_VerifyFinal(rsaCtx.get(), sig.data(), (unsigned int)sig.size(), pubKey.get())) {
            return false;
        }
        
        return true;
    }

#undef method

#define method StreamEncoder::

    method StreamEncoder(const std::string &aPublicKey, int offset)
    {
        // TODO: binary key
        auto publicKey = Encoder::base64decode(aPublicKey);

        int encMsgLen = 0;
        int blockLen  = 0;
        
        //EVP_PKEY *d2i_PrivateKey(int type, EVP_PKEY **a, unsigned char **pp, long length);
        
        auto remotePubKey = parsePublicKey(publicKey);
        
        // ek: encrypted key

        std::vector<unsigned char> ek(EVP_PKEY_size(remotePubKey.get()));
        std::vector<unsigned char> iv(EVP_MAX_IV_LENGTH);
        

        _rsaCtx = createContext();
        
        int ekl = 0;
        unsigned char *tmp2 = ek.data();
        auto tmp3 = remotePubKey.get();
        if(!EVP_SealInit(_rsaCtx.get(), EVP_aes_128_cbc(), &tmp2, &ekl, iv.data(), &tmp3, 1)) {
            _rsaCtx.reset();
            return;
        }
        uint32_t ekSize = htonl(ek.size());
        uint32_t ivSize = htonl(iv.size());
        unsigned char *ekSizeP = reinterpret_cast<unsigned char *>(&ekSize);
        unsigned char *ivSizeP = reinterpret_cast<unsigned char *>(&ivSize);
        
        _output.push_back(0);
        _output.insert(_output.end(), ekSizeP, ekSizeP + sizeof(uint32_t));
        _output.insert(_output.end(), ivSizeP, ivSizeP + sizeof(uint32_t));
        _output.insert(_output.end(), ek.begin(), ek.end());
        _output.insert(_output.end(), iv.begin(), iv.end());
    }

    void method encode(const unsigned char *ptr, int available)
    {
        if (!_rsaCtx) {
            return;
        }

        const unsigned char *data;
        int dataSize;
        // we only encode 16 byte long parts
        if (available % 16 == 0 && _input.empty()) {
            data = ptr;
            dataSize = available;
        } else {
            _input.insert(_input.end(), ptr, ptr+available);
            data = _input.data();
            dataSize = (_input.size() / 16) * 16;
        }

        int outputBefore = _output.size();
        _output.resize(outputBefore + dataSize + EVP_MAX_IV_LENGTH);

        int blockLen = 0;
        if(EVP_SealUpdate(_rsaCtx.get(), _output.data() + outputBefore, &blockLen, data, dataSize)) {
            
            _output.resize(outputBefore + blockLen);
        } else {
            _output.resize(outputBefore);
        }

        if (data != ptr) {
            _input.erase(_input.begin(), _input.begin() + dataSize);
        }
    }

    void method finalize()
    {
        if (_rsaCtx) {
            // padding
            if (_input.size() % 16 != 0) {
                _input.insert(_input.end(), 16 - (_input.size() % 16), 0);
            }
            if (_input.size() > 0) {
                int outputBefore = _output.size();
                _output.resize(outputBefore + _input.size() + EVP_MAX_IV_LENGTH);

                int blockLen = 0;
                if(EVP_SealUpdate(_rsaCtx.get(), _output.data() + outputBefore, &blockLen, _input.data(), _input.size())) {               
                    _output.resize(outputBefore + blockLen);
                } else {
                    _output.resize(outputBefore);
                }
            }

            int outputBefore = _output.size();
            _output.resize(outputBefore + EVP_MAX_IV_LENGTH);
            int blockLen = 0;
            if (EVP_SealFinal(_rsaCtx.get(), _output.data() + outputBefore, &blockLen)) {
                _output.resize(outputBefore + blockLen);
            } else {
                _output.resize(outputBefore);
            }

            _rsaCtx.reset();
        }
    }

#undef method

}

#endif

