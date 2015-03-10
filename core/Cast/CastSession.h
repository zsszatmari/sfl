//
//  CastSession.h
//  Cast
//
//  Created by Zsolt Szatmari on 03/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__CastSession__
#define __Cast__CastSession__

#include <string>
#include <functional>
#include "stdplus.h"
#include "json-forwards.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Cast
{
    class CastApplication;
    class CastDevice;
    
    using std::string;
    using std::function;
    
    class CastSession
    {
    public:
        static shared_ptr<CastSession> create(const shared_ptr<CastDevice> &device, const shared_ptr<CastApplication> &app, const string &namespace_id, const string &transportId);
    
        void send(const Json::Value &data, const function<bool(int, const Json::Value &)> &cb);
        
    private:
        CastSession(const shared_ptr<CastDevice> &device, const shared_ptr<CastApplication> &app, const string &namespace_id, const string &transportId);
        
        
        shared_ptr<CastDevice> _device;
        string _namespace;
        string _transportId;
    };
}

#endif /* defined(__Cast__CastSession__) */
