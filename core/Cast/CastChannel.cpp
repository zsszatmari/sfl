//
//  CastChannel.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 01/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include "CastChannel.h"
#include "json.h"
#include "Logger.h"

namespace Cast
{
#define method CastChannel::
    
    using namespace boost::asio;
    using namespace boost::asio::ip;
    
    shared_ptr<CastChannel> method create(boost::asio::io_service &io, boost::asio::ssl::context &sslContext, const string &senderId)
    {
        return shared_ptr<CastChannel>(new CastChannel(io, sslContext, senderId));
    }
    
    method CastChannel(boost::asio::io_service &io, boost::asio::ssl::context &sslContext, const string &senderId) :
        _socket(io,sslContext),
        _uniqueId(1),
        _jsonCast(senderId),
        _senderId(senderId)
    {
    }
    
    string method localHost() const
    {
        return _localHost;
    }
    
    void method connect(const string &host, uint16_t port, const function<void(int)> &handler)
    {
        tcp::endpoint destination;
        destination.port(port);
        destination.address(address::from_string(host));
        
        auto self = shared_from_this();
        self->_socket.set_verify_mode(boost::asio::ssl::context::verify_none);
        
        _socket.lowest_layer().async_connect(destination, [self,handler,host](boost::system::error_code ec){
            if (ec) {
                handler(ec.value());
                return;
            }
            self->_localHost = self->_socket.lowest_layer().local_endpoint().address().to_string();
            Base::Logger::stream() << "connected to chromecast at " << host << ". it can reach us at " << self->_localHost;
            
            self->_socket.async_handshake(boost::asio::ssl::stream_base::client, [self,handler](boost::system::error_code ec){
                std::cout << "handshake error " << ec << std::endl;
                if (ec) {
                    handler(ec.value());
                    return;
                }
                
                handler(0);
                self->readNext();
            });
        });
    }
    
    uint32_t method uniqueId()
    {
        return _uniqueId++;
    }
    
    void method send(const Json::Value &data, const function<bool(int,const Json::Value &)> &handler)
    {
        assert(handler);
        _handlers.insert(std::make_pair(0, handler));
        
        string dataDestination = data.get("destination","default").asString();
        
        if (_currentSession != dataDestination) {
            
            Json::Value payload(Json::objectValue);
            payload["namespace"] = "urn:x-cast:com.google.cast.tp.connection";
            if (dataDestination != "default") {
                payload["destination"] = dataDestination;
            }
            Json::Value innerData(Json::objectValue);
            innerData["type"] = "CONNECT";
            innerData["origin"] = Json::arrayValue;
            payload["data"] = innerData;
            
            write(payload);
            _currentSession = dataDestination;
        }
        
        auto datum = data;
        datum["data"]["requestId"] = uniqueId();
        
        write(datum);
    }
    
    void method write(const Json::Value &data)
    {
        CastMessage message = _jsonCast.encode(_authCast.encode(_heartCast.encode(data)));
        //std::cout << "sending: " << message.DebugString() << std::endl;
        vector<char> buffer = _plainCast.encode(message);
        if (buffer.empty()) {
            return;
        }
        auto self = shared_from_this();
        async_write(_socket, boost::asio::buffer(buffer.data(), buffer.size()), [self](boost::system::error_code ec, std::size_t length){
            //std::cout << "written " << length << " bytes\n";
            
            self->sendPending();
        });
    }
    
    void method readNext()
    {
        auto self = shared_from_this();
        _buffer.resize(1024);
        _socket.async_read_some(boost::asio::buffer(_buffer.data(), _buffer.size()), [self](boost::system::error_code ec, int bytes){
            
            if (ec) {
                std::cout << "error reading ChromeCast\n";
                return;
            }
            
            std::cout << "got " << bytes << " bytes from device!\n";
            vector<char> gotBytes(self->_buffer.begin(), self->_buffer.begin() + bytes);
            Json::Value decodedInter = self->_authCast.decode(self->_jsonCast.decode(self->_plainCast.decode(gotBytes)));
            Json::Value decoded = self->_heartCast.decode(decodedInter);
            
            
            Json::FastWriter writer;
            std::cout << "decoded as: " << writer.write(decoded) << std::endl;

            if (!decoded.empty()) {
                auto destination = decoded.get("destination","").asString();
                if (destination == self->_senderId || destination == "*") {

                    for (auto it = self->_handlers.begin() ; it != self->_handlers.end() ;) {
                        
                        // possibly invalidated iterator
                        auto currentIt = it++;
                        if (currentIt->second(0,decoded)) {
                            self->_handlers.erase(currentIt);
                        }
                    }
                }
            }
            
            self->sendPending();
            
            self->readNext();
        });
    }
    
    void method sendPending()
    {
        if (_authCast.pending() || _jsonCast.pending() || _plainCast.pending() || _heartCast.pending()) {
            write(Json::Value());
        }
    }
}