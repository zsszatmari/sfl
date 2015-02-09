//
//  CastSession.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 03/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "CastSession.h"
#include "json.h"
#include "CastDevice.h"
#include "ChromeCast.h"

namespace Cast
{
#define method CastSession::
    
    shared_ptr<CastSession> method create(const shared_ptr<CastDevice> &device, const shared_ptr<CastApplication> &app, const string &namespace_id, const string &transportId)
    {
        return shared_ptr<CastSession>(new CastSession(device, app, namespace_id, transportId));
    }
    
    method CastSession(const shared_ptr<CastDevice> &device, const shared_ptr<CastApplication> &app, const string &namespace_id, const string &transportId)  :
        _namespace(namespace_id),
        _transportId(transportId),
        _device(device)
    {
    }
    
    void method send(const Json::Value &data, const function<bool(int, const Json::Value &)> &cb)
    {
        Json::Value datum;
        datum["namespace"] = _namespace;
        datum["data"] = data;
        datum["destination"] = _transportId;
        
        auto device = _device;
        ChromeCast::instance()->io().dispatch([device,datum,cb]{
            device->channel()->send(datum, [cb](int ec, const Json::Value &answer){
                if (ec) {
                    cb(ec, Json::nullValue);
                    return true;
                } else {
                    return cb(0, answer.get("data", Json::objectValue));
                }
            });
        });
    }
}
