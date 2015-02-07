//
//  ChromeCast.h
//  Cast
//
//  Created by Zsolt Szatmari on 29/04/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__ChromeCast__
#define __Cast__ChromeCast__

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "stdplus.h"
#include SHAREDFROMTHIS_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Cast
{
    using std::string;
    using std::function;
 
    class CastDevice;
    
    class ChromeCast : public MEMORY_NS::enable_shared_from_this<ChromeCast>
    {
    public:
        static shared_ptr<ChromeCast> instance();
        
        void runSync();
        void probeAsync();
        void scanAsync();
        boost::asio::io_service &io();
        boost::asio::ssl::context &sslContext();
        void setFoundCallback(const function<void(const shared_ptr<CastDevice> &)> &foundCallback);
        
    private:
        void foundCandidate(const string &location);
        
        //int _socket;
        boost::asio::io_service _io;
        boost::asio::ssl::context _sslContext;
        boost::asio::ip::udp::socket _socket;
        std::vector<char> _readBuffer;
        boost::asio::ip::udp::endpoint _sender;
        function<void(const shared_ptr<CastDevice> &)> _foundCallback;
        
        ChromeCast();
        bool init();
        void readDescriptor(const string &host, const shared_ptr<boost::asio::ip::tcp::socket> &socket, size_t length = 0, shared_ptr<boost::asio::streambuf> buffer = nullptr);
    };
}

#endif /* defined(__Cast__ChromeCast__) */
