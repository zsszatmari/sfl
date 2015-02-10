//
//  AuthMessage.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 02/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <json.h>
#include <iostream>
#include "AuthMessage.h"
#include "cast_channel.pb.h"

namespace Cast
{
#define method AuthMessage::
 
    using std::string;
    using namespace extensions::api::cast_channel;
    
    method AuthMessage() :
        _authenticated(false),
        _inProgress(false)
    {
    }
    
    static const string urn = "urn:x-cast:com.google.cast.tp.deviceauth";
    
    Json::Value method decode(const Json::Value &chunk)
    {
        if (chunk["namespace"] == urn) {
            auto binary = chunk["binary"].asString();
            DeviceAuthMessage authMessage;
            bool success = authMessage.ParseFromString(binary);
            if (success) {
                std::cout << "authenticated\n";
                _authenticated = true;
            } else {
                std::cout << "auth failed\n";
            }
            return Json::Value();
        } else {
            return chunk;
        }
    }
    
    Json::Value method encode(const Json::Value &source)
    {
        if (!source.empty()) {
            _pendingEncode.push_back(source);
        }
        
//#define WAIT_FOR_AUTH_RESPONSE
#ifdef WAIT_FOR_AUTH_RESPONSE
        if (!_authenticated && _inProgress) {
            return Json::Value();
        }
#endif
        
        if (!_authenticated && !_inProgress) {
            _inProgress = true;
            DeviceAuthMessage auth;
            AuthChallenge *challenge = new AuthChallenge();
            bool init = challenge->IsInitialized();
            //challenge->InitAsDefaultInstance();
            auth.set_allocated_challenge(challenge);
            Json::Value ret(Json::objectValue);
            ret["namespace"] = urn;
            string data;
            auth.SerializeToString(&data);
            ret["binary"] = data;
            return ret;
        } else {
            if (!_pendingEncode.empty()) {
                auto ret = _pendingEncode.front();
                _pendingEncode.erase(_pendingEncode.begin());
                return ret;
            }
            return Json::Value();
        }
    }
    
    bool method pending()
    {
        return !_pendingEncode.empty();
    }
}
