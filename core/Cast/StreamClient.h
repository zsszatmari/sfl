//
//  StreamClient.h
//  Cast
//
//  Created by Zsolt Szatmari on 06/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__StreamClient__
#define __Cast__StreamClient__

#include <vector>
#include "boost/asio.hpp"
#include "stdplus.h"
#include SHAREDFROMTHIS_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Cast
{
    class StreamClient final : public MEMORY_NS::enable_shared_from_this<StreamClient>
    {
    public:
        static shared_ptr<StreamClient> create(boost::asio::ip::tcp::socket socket, const std::vector<char> &header);
        enum class State {
            Open,
            Stream,
            Discard
        };
        
        State state() const;
        // must be called from the right thread
        void write(const std::vector<char> &write, size_t offset);
        std::string ip() const;
        bool serveImage() const;
        
    private:
        StreamClient(boost::asio::ip::tcp::socket socket, const std::vector<char> &header);
        
        std::vector<char> _pendingBuffer;
        std::vector<char> _sendingBuffer;

        bool _serveImage;
        bool _firstLine;
        std::vector<char> _header;
        boost::asio::streambuf _buffer;
        void readRequest();
        void beginWrite();
        boost::asio::ip::tcp::socket _socket;
        // after _socket:
        std::string _ip;
        State _state;
        bool _writing;
    };
}

#endif /* defined(__Cast__StreamClient__) */
