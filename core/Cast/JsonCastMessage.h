//
//  JsonCastMessage.h
//  Cast
//
//  Created by Zsolt Szatmari on 02/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__JsonCastMessage__
#define __Cast__JsonCastMessage__

#include <vector>
#include"json-forwards.h"
#include "DataTransformer.h"
#include "cast_channel.pb.h"

namespace Cast
{
    using std::string;
    using std::vector;
    using namespace extensions::api::cast_channel;
    
    class JsonCastMessage : public DataTransformer
    {
    public:
        JsonCastMessage(const string &senderId);
        CastMessage encode(const Json::Value &source);
        Json::Value decode(const CastMessage &source);
        static string generateSenderId();
        
    private:
        string _name;
    };
}

#endif /* defined(__Cast__JsonCastMessage__) */
