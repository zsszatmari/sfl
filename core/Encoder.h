//
//  Encoder.h
//  receiptdaemon
//
//  Created by Zsolt Szatmari on 01/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __receiptdaemon__Encoder__
#define __receiptdaemon__Encoder__

#include <string>
#include <vector>
#include "openssl/evp.h"
#include "stdplus.h"
#include MEMORY_H

namespace Recd
{
    class Encoder final
    {
    public:
        //static std::vector<unsigned char> generateAesKey();
        static void generateKeys(int bits = 2048);
        static void generateBinaryKeys(int bits = 2048, int offset = 0);
        static std::pair<std::vector<unsigned char>, std::vector<unsigned char>> generatePublicPrivateKeys(int bits);
        
        static std::vector<unsigned char> base64decode(const std::string &str);
        static std::string base64encode(const std::vector<unsigned char> &data);
        
        static std::string seal(const std::string &msg, const std::vector<unsigned char> &publicKey);
        static std::string open(const std::string &msg, const std::vector<unsigned char> &privateKey);
        static std::string sign(const std::string &msg, const std::vector<unsigned char> &privateKey);
        static bool verifySignature(const std::string &msg, const std::vector<unsigned char> &publicKey, const std::string &signature);
    };

    class StreamEncoder final
    {
    public:
        StreamEncoder(const std::string &publicKey, int offset);
        
        void encode(const unsigned char *ptr, int available);
        void finalize();

        template<typename S>
        void write(S &stream)
        {
            stream.write(_output.data(), _output.size());
            _output.clear();
        }

    private:
        std::vector<unsigned char> _input;
        std::vector<unsigned char> _output;

        MEMORY_NS::shared_ptr<EVP_CIPHER_CTX> _rsaCtx;
    };
}

#endif /* defined(__receiptdaemon__Encoder__) */
