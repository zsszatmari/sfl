//
//  Device.h
//  Cast
//
//  Created by Zsolt Szatmari on 01/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__Device__
#define __Cast__Device__

#include <string>
#include "stdplus.h"
#include "CastChannel.h"

namespace Cast
{
    using std::string;
    
    class CastApplication;
    
    class CastDevice : public MEMORY_NS::enable_shared_from_this<CastDevice>
    {
    public:
        static shared_ptr<CastDevice> create(const std::string &name, const std::string &address);
        
        string name() const;
        string address() const;
        void application(const std::string &application_id, const function<void(int error, const shared_ptr<CastApplication> &app)> &handler);
        void connect(boost::asio::io_service &io, boost::asio::ssl::context &sslContext, const function<void(int)> &handler);
        
        shared_ptr<CastChannel> channel();
        shared_ptr<CastApplication> application() const;
        void stopApplication();
        void setApplication(const shared_ptr<CastApplication> &app);
        bool connected() const;
        
    private:
        CastDevice(const std::string &name, const std::string &address);
        shared_ptr<CastChannel> _channel;
        // we must retain this for continued playback state updates
        shared_ptr<CastApplication> _application;
        
        std::string _name;
        std::string _address;
        bool _connected;
    };
}

#endif /* defined(__Cast__Device__) */
