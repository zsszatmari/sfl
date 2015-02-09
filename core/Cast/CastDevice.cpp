//
//  Device.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 01/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include "Json.h"
#include "CastDevice.h"
#include "CastApplication.h"
#include "IApp.h"
#include "IPlaybackWorker.h"

namespace Cast
{
#define method CastDevice::
    
    shared_ptr<CastDevice> method create(const std::string &name, const std::string &address)
    {
        return shared_ptr<CastDevice>(new CastDevice(name, address));
    }
    
    method CastDevice(const std::string &name, const std::string &address) :
        _name(name),
        _address(address)
    {
    }
    
    string method name() const
    {
        return _name;
    }
    
    string method address() const
    {
        return _address;
    }
    
    void method connect(boost::asio::io_service &io, boost::asio::ssl::context &sslContext, const function<void(int)> &handler)
    {
        auto senderId = JsonCastMessage::generateSenderId();
        std::cout << "sender id: " << senderId << std::endl;
        _channel = CastChannel::create(io, sslContext, senderId);
        _channel->connect(_address, 8009, handler);
    }
    
    void method application(const std::string &application_id, const function<void(int error, const shared_ptr<CastApplication> &app)> &handler)
    {
        _connected = true;
        Json::Value msg(Json::objectValue);
        msg["namespace"] = "urn:x-cast:com.google.cast.receiver";
        Json::Value data(Json::objectValue);
        data["type"] = "GET_APP_AVAILABILITY";
        Json::Value appId(Json::arrayValue);
        appId[0] = application_id;
        data["appId"] = appId;
        msg["data"] = data;
        auto self = shared_from_this();
        
        _channel->send(msg, [application_id,handler,self](int error, const Json::Value &result){
            
            std::cout << "some reply\n";
            handler(0, CastApplication::create(application_id, self->_channel, self));
            return true;
        });

        weak_ptr<CastDevice> wself = self;


        // this leads to premature termination:
        _channel->send(Json::Value(), [wself](int error, const Json::Value &result) {
            // destination must be checked by the channel itself
            if (result.get("data",Json::objectValue).get("type","").asString() == "CLOSE") {
                std::cout << "ChromeCast disconnect\n";
                auto s = wself.lock();
                if (s) {
                    s->stopApplication();
                }
                return true;
            }
            return false;
        });
    }
    
    bool method connected() const
    {
        return _connected;
    }

    shared_ptr<CastChannel> method channel()
    {
        return _channel;
    }

    shared_ptr<CastApplication> method application() const
    {
        return _application;
    }

    void method setApplication(const shared_ptr<CastApplication> &app)
    {
        _application = app;
    }

    void method stopApplication()
    {
        _connected = false;
        _application.reset();
        Gear::IApp::instance()->playbackWorker()->reload();
    }
}