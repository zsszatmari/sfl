//
//  PlainCastMessage.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 02/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif
#include <stdint.h>
#include "PlainCastMessage.h"

namespace Cast
{
    using std::string;
    
#define method PlainCastMessage::
    
    vector<char> method encode(const CastMessage &message)
    {
        if (!message.IsInitialized()) {
            return vector<char>();
        }
        
        string source;
        message.SerializeToString(&source);
        
        vector<char> ret;
        ret.resize(4);
        uint32_t *pLen = (uint32_t *)&ret[0];
        *pLen = htonl(source.size());
        ret.insert(ret.end(), source.begin(), source.end());
        return ret;
    }
    
    CastMessage method decode(const vector<char> &source)
    {
        _gathered.insert(_gathered.end(), source.begin(), source.end());
        if (_gathered.size() < 4) {
            return CastMessage();
        }
        uint32_t *pSize = (uint32_t *)&_gathered[0];
        uint32_t size = ntohl(*pSize);
        
        if (_gathered.size() < 4 + size) {
            return CastMessage();
        }
        
        string ret(_gathered.begin() + 4, _gathered.begin() +4+size);
        _gathered.erase(_gathered.begin(), _gathered.begin() +4+size);
        CastMessage message;
        if (!message.ParseFromString(ret)) {
            setError(1);
        }
        
        return message;
    }
        
#undef method
}