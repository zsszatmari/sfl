//
//  GooglePlaySignature.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/26/13.
//
//

#include <algorithm>
#include "GooglePlaySignature.h"
#include "GearUtility.h"
#include "HMAC_SHA1.h"
#include "base64.h"

namespace Gear
{
#define method GooglePlaySignature::
    
    method GooglePlaySignature() :
        _salt(random_string(12))
    {
    }
    
    static const string hmacSha1Digest(const string &key, const string &value)
    {
        unsigned char digest[CHMAC_SHA1::SHA1_DIGEST_LENGTH];
        CHMAC_SHA1 HMAC_SHA1;
        HMAC_SHA1.HMAC_SHA1((unsigned char *)value.data(), (int)value.size(), (unsigned char *)key.data(), (int)key.size(), digest);
        
        return string((char *)digest, sizeof(digest));
    }
    
    static const string base64(const string &str)
    {
        size_t length = 0;
        char *base64c = base64_encode(reinterpret_cast<const unsigned char *>(str.data()), str.length(), &length);
        const string ret(base64c, length);
        free(base64c);
        
        return ret;
    }
    
    const string method generateForSongId(const string &songId) const
    {
        //sig = binascii.b2a_base64(hmac.new(key, (songid + salt), sha1).digest())[:-1].replace('+', '-').replace('/', '_').replace('=', '.')
        
        const string key = "27f7313e-f75d-445a-ac99-56386a5fe879";
        
        
        const string hmacDigest = hmacSha1Digest(key, (songId + salt()));
        string encoded = base64(hmacDigest);
        std::replace(encoded.begin(), encoded.end(), '+', '-');
        std::replace(encoded.begin(), encoded.end(), '/', '_');
        std::replace(encoded.begin(), encoded.end(), '=', '.');
        
        return encoded;
    }
    
    const string & method salt() const
    {
        return _salt;
    }
    
    void method setSalt(const string &salt)
    {
        _salt = salt;
    }
}