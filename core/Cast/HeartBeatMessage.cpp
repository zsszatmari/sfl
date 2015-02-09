//
//  HeartbeatMessage.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 03/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "HeartBeatMessage.h"
#include "json.h"

namespace Cast
{
#define method HeartBeatMessage::
    
    using std::string;
    
    Json::Value method decode(const Json::Value &source)
    {
        string urn("urn:x-cast:com.google.cast.tp.heartbeat");
        if (source.get("namespace","").asString() == urn) {
            string type = source.get("data",Json::objectValue).get("type","").asString();
            if (type == "PING") {
                Json::Value emit(Json::objectValue);
                emit["namespace"] = urn;
                emit["source"] = source.get("destination","");
                emit["destination"] = source.get("source","");
                Json::Value data(Json::objectValue);
                data["type"] = "PONG";
                emit["data"] = data;
                _pendingEncode.push_back(emit);
                
                return Json::Value();
            } else if (type == "PONG") {
                // we are happy
                return Json::Value();
            }
        }
        return source;
    }
    
    Json::Value method encode(const Json::Value &source)
    {
        if (!source.empty()) {
            _pendingEncode.push_back(source);
        }
        // we might send heartbeats here...
        
        if (!_pendingEncode.empty()) {
            auto send = _pendingEncode.front();
            _pendingEncode.erase(_pendingEncode.begin());
            return send;
        }
        return Json::Value();
    }
    
    bool method pending()
    {
        return !_pendingEncode.empty();
    }
}
