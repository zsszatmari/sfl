//
//  JsonCastMessage.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 02/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include "JsonCastMessage.h"
#include "json.h"
#include "stdplus.h"
#include CHRONO_H

#ifdef USE_BOOST_INSTEAD_OF_CPP11
#include "boost/random.hpp"
typedef boost::random::minstd_rand default_random_engine;
using boost::random::uniform_int_distribution;
#else
#include <random>
using std::default_random_engine;
using std::uniform_int_distribution;
#endif

namespace Cast
{
#define method JsonCastMessage::
    
    static string randomString(size_t len)
    {
        static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        
        string out;
        static default_random_engine generator(CHRONO_NS::system_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> distribution(0,sizeof(alphanum)-1-1);
        
        for (int i = 0; i < len; ++i) {
            out += alphanum[distribution(generator)];
        }
        
        return out;
    }
    
    string method generateSenderId()
    {
        return "sender-" + randomString(10);
    }

    method JsonCastMessage(const string &senderId) :
        _name(senderId)
    {
    }
    
    CastMessage method encode(const Json::Value &chunk)
    {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        
        if (chunk.empty()) {
            return CastMessage();
        }
        
        Json::FastWriter writer;
        std::cout << "raw send: " << writer.write(chunk) << std::endl;
        
        CastMessage message;
        message.set_protocol_version(CastMessage_ProtocolVersion_CASTV2_1_0);
        message.set_source_id(chunk.get("source", _name).asString());
        message.set_destination_id(chunk.get("destination", "receiver-0").asString());
        message.set_namespace_(chunk.get("namespace","").asString());
        
        if (chunk.isMember("binary")) {
            message.set_payload_type(CastMessage_PayloadType_BINARY);
            message.set_payload_binary(chunk["binary"].asString().c_str(), chunk["binary"].asString().length() +1);
            message.set_payload_binary(chunk["binary"].asString());
        } else {
            message.set_payload_type(CastMessage_PayloadType_STRING);
            Json::FastWriter writer;
            message.set_payload_utf8(writer.write(chunk["data"]));
        }
        return message;
    }

    
    Json::Value method decode(const CastMessage &chunk)
    {
        if (!chunk.IsInitialized()) {
            return Json::Value();
        }
        
        if (chunk.protocol_version() != CastMessage_ProtocolVersion_CASTV2_1_0) {
            setError(1);
            return Json::Value();
        }
        Json::Value result;
        result["source"] = chunk.source_id();
        result["destination"] = chunk.destination_id();
        result["namespace"] = chunk.namespace_();
        if (chunk.payload_type() == CastMessage_PayloadType_BINARY) {
            result["binary"] = chunk.payload_binary();
        } else {
            auto payload = chunk.payload_utf8();
            if (payload.size() > 0 && payload.at(0) == '{') {
                Json::Value value;
                Json::Reader parser;
                parser.parse(payload, value);
                result["data"] = value;
            } else {
                result["data"] = payload;
            }
        }
        
        Json::FastWriter writer;
        std::cout << "raw decoded as: " << writer.write(result) << std::endl;
        
        return result;
    }
        
#undef method
}