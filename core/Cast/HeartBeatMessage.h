//
//  HeartbeatMessage.h
//  Cast
//
//  Created by Zsolt Szatmari on 03/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__HeartbeatMessage__
#define __Cast__HeartbeatMessage__

#include <vector>
#include "DataTransformer.h"
#include "json-forwards.h"

namespace Cast
{
    using std::vector;
    
    class HeartBeatMessage : public DataTransformer
    {
    public:
        Json::Value decode(const Json::Value &source);
        Json::Value encode(const Json::Value &source);
        virtual bool pending();
    
    private:
        vector<Json::Value> _pendingEncode;
    };
}

#endif /* defined(__Cast__HeartbeatMessage__) */
