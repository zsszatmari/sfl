//
//  Streamer.h
//  Cast
//
//  Created by Zsolt Szatmari on 06/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __Cast__Streamer__
#define __Cast__Streamer__

#include <vector>
#include "stdplus.h"
#include "StreamClient.h"
#include "boost/asio.hpp"
#include "vorbis/vorbisenc.h"
#include "vorbis/codec.h"
#include "AtomicPtr.h"
#include WEAK_H
using MEMORY_NS::weak_ptr;

namespace Gear
{
    class CastPlaybackWorker;
}

namespace Cast
{
    using namespace Gear;

    class Streamer final
    {
    public:
        Streamer(boost::asio::io_service &io, const shared_ptr<CastPlaybackWorker> &worker);
        ~Streamer();
        
        uint16_t port() const;
        
        // return starting offset. meaningful data begins at kMaxPageHeaderSize by convention
        static uint16_t generateHeader(std::vector<char> &data, const std::vector<int> &packetSizes, const uint32_t sequence, const uint64_t granule, uint8_t headerType);
        
        void writeSamplesFromIo(const float *buf, size_t bufSize);
        void clearBuffer();
        void streamSilence();
        void cancelStreaming();

    private:
        Streamer &operator=(const Streamer &); // delete
        Streamer(const Streamer &); // delete
        
        void doAccept();
        
        size_t calculateUseSamples(size_t samples);
        static void int_to_char(uint32_t i, unsigned char ch[4]);
        
        boost::asio::io_service &_io;
        boost::asio::ip::tcp::acceptor _acceptor;
        boost::asio::ip::tcp::socket _acceptSocket;
        uint16_t _port;
        uint32_t _sequence;
        vorbis_info _info;
        vorbis_dsp_state _dsp;
        vorbis_comment _comment;
        vorbis_block _block;
        
        // +2 to match boundaries (speed)
        static const int kMaxPageHeaderSize = 27+255 +2;

        const shared_ptr<std::vector<char>> _header;
        std::vector<char> _page;
        bool _buffering;

        weak_ptr<CastPlaybackWorker> _worker;
        std::vector<shared_ptr<StreamClient>> _clients;
        AtomicPtr<std::vector<std::pair<int,shared_ptr<std::vector<char>>>>> _preBuffer;
    };
    
#define method Streamer::
    
#undef method
}

#endif /* defined(__Cast__Streamer__) */
