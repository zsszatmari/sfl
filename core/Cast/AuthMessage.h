//
//  AuthMessage.h
//  Cast
//
//  Created by Zsolt Szatmari on 02/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__AuthMessage__
#define __Cast__AuthMessage__

#include <vector>
#include "json-forwards.h"
#include "DataTransformer.h"

namespace Cast
{
    using std::vector;
    
    class AuthMessage
    {
    public:
        AuthMessage();
        Json::Value decode(const Json::Value &source);
        Json::Value encode(const Json::Value &source);
        virtual bool pending();
        
    private:
        bool _authenticated;
        bool _inProgress;
        vector<Json::Value> _pendingEncode;
    };
}

#endif /* defined(__Cast__AuthMessage__) */
