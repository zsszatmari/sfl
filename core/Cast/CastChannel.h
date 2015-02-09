//
//  CastChannel.h
//  Cast
//
//  Created by Zsolt Szatmari on 01/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__CastChannel__
#define __Cast__CastChannel__

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "stdplus.h"
#include "json-forwards.h"
#include "AuthMessage.h"
#include "JsonCastMessage.h"
#include "PlainCastMessage.h"
#include "HeartBeatMessage.h"
#include SHAREDFROMTHIS_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Cast
{
    using std::string;
    using std::multimap;
    using std::function;
    
    class CastChannel final : public MEMORY_NS::enable_shared_from_this<CastChannel>
    {
    public:
        static shared_ptr<CastChannel> create(boost::asio::io_service &io, boost::asio::ssl::context &sslContext, const string &senderId);
        
        void connect(const string &host, uint16_t port, const function<void(int)> &handler);
        // true return value means acquired, erase handler
        void send(const Json::Value &data, const function<bool(int,const Json::Value &)> &);
        
        std::string localHost() const;
        
    private:
        CastChannel(boost::asio::io_service &io, boost::asio::ssl::context &sslContext, const string &senderId);
        
        void write(const Json::Value &data);
        void readNext();
        void sendPending();
        
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> _socket;
        string _currentSession;
        uint32_t uniqueId();
        
        uint32_t _uniqueId;
    
        AuthMessage _authCast;
        JsonCastMessage _jsonCast;
        PlainCastMessage _plainCast;
        HeartBeatMessage _heartCast;
        
        typedef int time;
        // TODO: clear these and send timeout to handler
        multimap<time, function<bool(int,const Json::Value &)>> _handlers;
        vector<char> _buffer;
        
        std::string _localHost;
        const std::string _senderId;
        
        CastChannel &operator=(const CastChannel &); // delete
        CastChannel(const CastChannel &); //delete
    };
}

#endif /* defined(__Cast__CastChannel__) */
