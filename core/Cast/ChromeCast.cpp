//
//  ChromeCast.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 29/04/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <string.h>
#include <string>
#include <stdio.h>
#include "ChromeCast.h"
#include "CastDevice.h"
#ifdef __APPLE__
#ifndef NON_APPLE
#define APPLE_CHROMECAST
#endif
#endif

#ifdef APPLE_CHROMECAST
#include "ChromeCastApple.h"
#endif

namespace Cast
{
#define method ChromeCast::
 
    using namespace boost::asio;
    using namespace boost::asio::ip;
    using std::string;
    using std::stringstream;
    
    
    shared_ptr<ChromeCast> method instance()
    {
        static shared_ptr<ChromeCast> inst(new ChromeCast());
        return inst;
    }
    
    method ChromeCast() :
        _socket(_io),
        //_sslContext(boost::asio::ssl::context::tlsv12_client)
        _sslContext(boost::asio::ssl::context::sslv23_client)
    {
        init();
    }
    
    boost::asio::ssl::context & method sslContext()
    {
        return _sslContext;
    }
    
    bool method init()
    {
        _socket.open(udp::v4());
        _socket.set_option(boost::asio::socket_base::broadcast(true));
        return true;
    }
    
    void method runSync()
    {
        //probeAsync();
        //scanAsync();

#ifdef _WIN32
#pragma message("TODO: make ChromeCast work on windows")
        return;
#endif
        _io.run();
    }
    
    void method probeAsync()
    {
#ifdef APPLE_CHROMECAST
        static shared_ptr<ChromeCastApple> chromeCastApple(new ChromeCastApple());
        chromeCastApple->probeAsync(_foundCallback);
        
        // we dneed both versions of discovery running parallel to each other
        // the SSDP version has problems at Chris Gross
        // the other has problems sometimes at other people's, apple losing track of devices
        // and os x needs to be restarted
        //return;
#endif
        const std::string BROADCAST_ADDRESS = "239.255.255.250";
        // this does not work...:
        //const std::string BROADCAST_ADDRESS = "239.255.255.255";
        const std::string BROADCAST_SERVER_PORT = "1900";
        const std::string SEARCH_TARGET = "urn:dial-multiscreen-org:service:dial:1";
        const std::string packet = std::string("M-SEARCH * HTTP/1.1\r\n") + "HOST: " + BROADCAST_ADDRESS + ":" + BROADCAST_SERVER_PORT + "\r\n"
        + "MAN: \"ssdp:discover\"\r\n" + "MX: 10\r\n" + "ST: " + SEARCH_TARGET + "\r\n\r\n";

        udp::endpoint destination;
        //destination.address(address::from_string("255.255.255.255"));
        destination.address(address::from_string(BROADCAST_ADDRESS));
        destination.port(1900);
        
        _socket.async_send_to(boost::asio::buffer(packet), destination, [](boost::system::error_code ec, std::size_t length){
            //std::cout << "sent probe data " << length << std::endl;
        });
    }
    
    void method scanAsync()
    {
#ifdef APPLE_CHROMECAST
        //return;
#endif
        _readBuffer.resize(4096);
        _socket.async_receive_from(boost::asio::buffer(_readBuffer, _readBuffer.size()), _sender, [&](boost::system::error_code ec, std::size_t length){
            
            if (ec) {
                scanAsync();
                return;
            }
            
            std::string str(_readBuffer.begin(), _readBuffer.begin() + length);
            std::stringstream ss(str);
            std::string line;
            
            string location;
            while(true) {
                line.clear();
                std::getline(ss,line);
                if (line.empty()) {
                    break;
                }
                std::string prefix("LOCATION: http://");
                if (line.compare(0, prefix.size(), prefix) == 0) {
                    // strip trailing \r
                    location = line.substr(prefix.size(), line.size() -1 - prefix.size());
                    break;
                }
            }
            if (!location.empty()) {
                foundCandidate(location);
            }
            
            scanAsync();
        });
    }
    
    void method foundCandidate(const std::string &location)
    {
        string colon(":");
        string slash("/");
        
        auto colonPos = location.find(colon);
        auto slashPos = location.find(slash);
        if (slashPos == string::npos || colonPos == string::npos) {
            return;
        }
        
        auto host = location.substr(0, colonPos);
        auto port = location.substr(colonPos+1, slashPos - (colonPos+1));
        auto path = location.substr(slashPos);
            
        shared_ptr<boost::asio::ip::tcp::socket> socket(new boost::asio::ip::tcp::socket(_io));
        tcp::endpoint destination;
        destination.port(atoi(port.c_str()));
        destination.address(address::from_string(host));
        
        socket->async_connect(destination, [=](boost::system::error_code ec){
            if (ec) {
                return;
            }
            stringstream ss;
            ss << "GET " << path << " HTTP/1.1\r\n";
            ss << "Host: " << host << ":" << port << "\r\n";
            ss << "Connection: close\r\n";
            ss << "Accept-Encoding: *\r\n";
            ss << "\r\n";
            const std::string sendData = ss.str();
            async_write(*socket, boost::asio::buffer(sendData), [=](boost::system::error_code ec, int len){
                if (ec) {
                    return;
                }
                
                readDescriptor(host, socket);
            });
        });
    }
    
    void method readDescriptor(const string &host, const shared_ptr<boost::asio::ip::tcp::socket> &socket, size_t length, shared_ptr<boost::asio::streambuf> buffer)
    {
        if (!buffer) {
            buffer = shared_ptr<boost::asio::streambuf>(new boost::asio::streambuf());
        }
        auto self = shared_from_this();
        async_read_until(*socket, *buffer, '\n', [self,socket,buffer,length,host](boost::system::error_code ec, std::size_t readLength) mutable {
            
            if (ec) {
                return;
            }
            
            std::istream is(buffer.get());
            std::string line;
            std::getline(is, line);
            
            const string lengthPrefix("Content-Length:");
            if (line.size() > lengthPrefix.size() && line.compare(0, lengthPrefix.size(), lengthPrefix) == 0) {
                string lengthStr = line.substr(lengthPrefix.size());
                length = atoi(lengthStr.c_str());
            }
            
            if (line == "\r") {
                if (length == 0) {
                    return;
                }
                std::string xmlData((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
                shared_ptr<std::vector<char>> xmlBuf(new std::vector<char>(length - xmlData.size()));
                
                async_read(*socket, boost::asio::buffer(*xmlBuf, xmlBuf->size()), [self,socket,buffer,xmlData,xmlBuf,host](boost::system::error_code ec, int bytes) mutable {
                    if (ec) {
                        return;
                    }
                    xmlData.insert(xmlData.end(), xmlBuf->begin(), xmlBuf->begin()+bytes);
                    
                    auto getTag = [&](const string &tag)->string{
                        const string kPrefix = "<" + tag + ">";
                        const string kPostfix = "</" + tag + ">";
                        auto prepos = xmlData.find(kPrefix);
                        if (prepos == string::npos) {
                            return "";
                        }
                        prepos += kPrefix.length();
                        auto postpos = xmlData.find(kPostfix, prepos);
                        if (postpos == string::npos) {
                            return "";
                        }
                        return xmlData.substr(prepos, postpos-prepos);
                    };
                    
                    if (getTag("modelName") != "Eureka Dongle") {
                        return;
                    }
                    auto name = getTag("friendlyName");
                    if (name.empty()) {
                        return;
                    }
                    auto device = CastDevice::create(name, host);
                    
                    std::cout << "found: '" << name << "' at " << host << std::endl;
                    if (self->_foundCallback) {
                        self->_foundCallback(device);
                    }
                });
            } else {
                self->readDescriptor(host, socket, length, buffer);
            }
        });
    }
    
    void method setFoundCallback(const function<void(const shared_ptr<CastDevice> &)> &foundCallback)
    {
        _foundCallback = foundCallback;
    }
    
    boost::asio::io_service & method io()
    {
        return _io;
    }
}
