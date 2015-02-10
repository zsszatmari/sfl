//
//  Streamer.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 06/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include "Streamer.h"
#include "stdplus.h"
#include "StreamClient.h"
#include "IApp.h"
#include "IPlayer.h"
#include "CastPlaybackWorker.h"

namespace Cast
{
#define method Streamer::
    
    using namespace Gear;
    using namespace boost::asio;
    using namespace boost::asio::ip;
    using std::vector;
    
    method Streamer(boost::asio::io_service &io, const shared_ptr<CastPlaybackWorker> &worker) :
        _io(io),
        _acceptor(io),
        _acceptSocket(io),
        _sequence(2),
        _header(new vector<char>()),
        _buffering(true),
        _worker(worker)
    {
        vorbis_info_init(&_info);
        vorbis_encode_setup_vbr(&_info, 2, 44100, 1.0f);
        vorbis_encode_setup_init(&_info);
        
        vorbis_analysis_init(&_dsp, &_info);
        vorbis_comment_init(&_comment);
        
        ogg_packet op_ident;
        ogg_packet op_comm;
        ogg_packet op_code;
        
        vorbis_analysis_headerout(&_dsp, &_comment, &op_ident, &op_comm, &op_code);
        int size1 = op_ident.bytes;
        int size2 = op_comm.bytes;
        int size3 = op_code.bytes;
        
        auto &header = *_header;
        {
            vector<char> page;
            page.resize(kMaxPageHeaderSize);
            page.insert(page.end(), op_ident.packet, op_ident.packet + op_ident.bytes);
            vector<int> sizes;
            sizes.push_back(op_ident.bytes);
            
            int offset = generateHeader(page, sizes, 0, 0, 2);
            header.insert(header.end(), page.begin()+offset, page.end());
        }
        {
            vector<char> page;
            vector<int> sizes;
            page.resize(kMaxPageHeaderSize);
            page.insert(page.end(), op_comm.packet, op_comm.packet + op_comm.bytes);
            sizes.push_back(op_comm.bytes);
            page.insert(page.end(), op_code.packet, op_code.packet + op_code.bytes);
            sizes.push_back(op_code.bytes);
            
            int offset = generateHeader(page, sizes, 1, 0, 0);
            header.insert(header.end(), page.begin()+offset, page.end());
        }
        
        /*ogg_packet_clear(&op_ident);
        ogg_packet_clear(&op_comm);
        ogg_packet_clear(&op_code);
        */
        vorbis_block_init(&_dsp, &_block);
        
        unsigned short port(0);
        tcp::endpoint endPoint(tcp::endpoint(tcp::v4(), port));
        _acceptor.open(endPoint.protocol());
        _acceptor.set_option(tcp::acceptor::reuse_address(true));
        _acceptor.bind(endPoint);
        
        _acceptor.listen();
        endPoint = _acceptor.local_endpoint();
        _port = endPoint.port();
        // this gives 0.0.0.0
        //std::string addr = endPoint.address().to_string();
        std::cout << "using port " << _port << std::endl;
        
        doAccept();
    }
    
    method ~Streamer()
    {
        vorbis_block_clear(&_block);
        vorbis_comment_clear(&_comment);
        vorbis_dsp_clear(&_dsp);
        vorbis_info_clear(&_info);
        //opus_encoder_destroy(_encoder);
    }
    
    uint16_t method port() const
    {
        return _port;
    }
    
    void method doAccept()
    {
        _acceptor.async_accept(_acceptSocket, [this](boost::system::error_code ec){

            /*
            boost::error_code setEc;
            {
                boost::asio::ip::tcp::no_delay option(true);
                socket.set_option(option, setEc);
            }
            boost::asio::ip::tcp::no_delay option;
            bool isSet = socket.get_option();
            */



            vector<char> header;
            size_t s = _header->size();
            std::vector<std::pair<int,shared_ptr<std::vector<char>>>> preBuffer = _preBuffer;
            for (auto &p : preBuffer) {
                s += p.second->size() - p.first;
            }
            header.reserve(s);
            header.insert(header.end(), _header->begin(), _header->end());
            for (auto &p : preBuffer) {
                header.insert(header.end(), p.second->begin() + p.first, p.second->end());
            }
            _clients.push_back(StreamClient::create(std::move(_acceptSocket), header));
            doAccept();
        });
    }
    
    void method int_to_char(uint32_t i, unsigned char ch[4])
    {
        ch[0] = i>>24;
        ch[1] = (i>>16)&0xFF;
        ch[2] = (i>>8)&0xFF;
        ch[3] = i&0xFF;
    }

    static const uint32_t crc_lookup[256]={
        0x00000000,0x04c11db7,0x09823b6e,0x0d4326d9,
        0x130476dc,0x17c56b6b,0x1a864db2,0x1e475005,
        0x2608edb8,0x22c9f00f,0x2f8ad6d6,0x2b4bcb61,
        0x350c9b64,0x31cd86d3,0x3c8ea00a,0x384fbdbd,
        0x4c11db70,0x48d0c6c7,0x4593e01e,0x4152fda9,
        0x5f15adac,0x5bd4b01b,0x569796c2,0x52568b75,
        0x6a1936c8,0x6ed82b7f,0x639b0da6,0x675a1011,
        0x791d4014,0x7ddc5da3,0x709f7b7a,0x745e66cd,
        0x9823b6e0,0x9ce2ab57,0x91a18d8e,0x95609039,
        0x8b27c03c,0x8fe6dd8b,0x82a5fb52,0x8664e6e5,
        0xbe2b5b58,0xbaea46ef,0xb7a96036,0xb3687d81,
        0xad2f2d84,0xa9ee3033,0xa4ad16ea,0xa06c0b5d,
        0xd4326d90,0xd0f37027,0xddb056fe,0xd9714b49,
        0xc7361b4c,0xc3f706fb,0xceb42022,0xca753d95,
        0xf23a8028,0xf6fb9d9f,0xfbb8bb46,0xff79a6f1,
        0xe13ef6f4,0xe5ffeb43,0xe8bccd9a,0xec7dd02d,
        0x34867077,0x30476dc0,0x3d044b19,0x39c556ae,
        0x278206ab,0x23431b1c,0x2e003dc5,0x2ac12072,
        0x128e9dcf,0x164f8078,0x1b0ca6a1,0x1fcdbb16,
        0x018aeb13,0x054bf6a4,0x0808d07d,0x0cc9cdca,
        0x7897ab07,0x7c56b6b0,0x71159069,0x75d48dde,
        0x6b93dddb,0x6f52c06c,0x6211e6b5,0x66d0fb02,
        0x5e9f46bf,0x5a5e5b08,0x571d7dd1,0x53dc6066,
        0x4d9b3063,0x495a2dd4,0x44190b0d,0x40d816ba,
        0xaca5c697,0xa864db20,0xa527fdf9,0xa1e6e04e,
        0xbfa1b04b,0xbb60adfc,0xb6238b25,0xb2e29692,
        0x8aad2b2f,0x8e6c3698,0x832f1041,0x87ee0df6,
        0x99a95df3,0x9d684044,0x902b669d,0x94ea7b2a,
        0xe0b41de7,0xe4750050,0xe9362689,0xedf73b3e,
        0xf3b06b3b,0xf771768c,0xfa325055,0xfef34de2,
        0xc6bcf05f,0xc27dede8,0xcf3ecb31,0xcbffd686,
        0xd5b88683,0xd1799b34,0xdc3abded,0xd8fba05a,
        0x690ce0ee,0x6dcdfd59,0x608edb80,0x644fc637,
        0x7a089632,0x7ec98b85,0x738aad5c,0x774bb0eb,
        0x4f040d56,0x4bc510e1,0x46863638,0x42472b8f,
        0x5c007b8a,0x58c1663d,0x558240e4,0x51435d53,
        0x251d3b9e,0x21dc2629,0x2c9f00f0,0x285e1d47,
        0x36194d42,0x32d850f5,0x3f9b762c,0x3b5a6b9b,
        0x0315d626,0x07d4cb91,0x0a97ed48,0x0e56f0ff,
        0x1011a0fa,0x14d0bd4d,0x19939b94,0x1d528623,
        0xf12f560e,0xf5ee4bb9,0xf8ad6d60,0xfc6c70d7,
        0xe22b20d2,0xe6ea3d65,0xeba91bbc,0xef68060b,
        0xd727bbb6,0xd3e6a601,0xdea580d8,0xda649d6f,
        0xc423cd6a,0xc0e2d0dd,0xcda1f604,0xc960ebb3,
        0xbd3e8d7e,0xb9ff90c9,0xb4bcb610,0xb07daba7,
        0xae3afba2,0xaafbe615,0xa7b8c0cc,0xa379dd7b,
        0x9b3660c6,0x9ff77d71,0x92b45ba8,0x9675461f,
        0x8832161a,0x8cf30bad,0x81b02d74,0x857130c3,
        0x5d8a9099,0x594b8d2e,0x5408abf7,0x50c9b640,
        0x4e8ee645,0x4a4ffbf2,0x470cdd2b,0x43cdc09c,
        0x7b827d21,0x7f436096,0x7200464f,0x76c15bf8,
        0x68860bfd,0x6c47164a,0x61043093,0x65c52d24,
        0x119b4be9,0x155a565e,0x18197087,0x1cd86d30,
        0x029f3d35,0x065e2082,0x0b1d065b,0x0fdc1bec,
        0x3793a651,0x3352bbe6,0x3e119d3f,0x3ad08088,
        0x2497d08d,0x2056cd3a,0x2d15ebe3,0x29d4f654,
        0xc5a92679,0xc1683bce,0xcc2b1d17,0xc8ea00a0,
        0xd6ad50a5,0xd26c4d12,0xdf2f6bcb,0xdbee767c,
        0xe3a1cbc1,0xe760d676,0xea23f0af,0xeee2ed18,
        0xf0a5bd1d,0xf464a0aa,0xf9278673,0xfde69bc4,
        0x89b8fd09,0x8d79e0be,0x803ac667,0x84fbdbd0,
        0x9abc8bd5,0x9e7d9662,0x933eb0bb,0x97ffad0c,
        0xafb010b1,0xab710d06,0xa6322bdf,0xa2f33668,
        0xbcb4666d,0xb8757bda,0xb5365d03,0xb1f740b4};
    
    static uint32_t
    crc32(uint32_t crc_reg, const uint8_t *buf, size_t size)
    {
        for (int i = 0 ; i < size ; i++) {
            crc_reg=(crc_reg<<8)^crc_lookup[((crc_reg >> 24)&0xff)^buf[i]];
        }
        return crc_reg;
    }

    
    uint16_t method generateHeader(std::vector<char> &data, const std::vector<int> &packetSizes, const uint32_t sequence, const uint64_t granule, uint8_t headerType)
    {
#if 0
        unsigned char test[] = {0x4f, 0x67, 0x67, 0x53, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x5b,
                                0xf2, 0x35, 0x00, 0x00, 0x00, 0x00, /*0xc2, 0x44, 0x10, 0x52*/0,0,0,0, 0x01, 0x13, 0x4f, 0x70, 0x75, 0x73,
                                0x48, 0x65, 0x61, 0x64, 0x01, 0x02, 0x64, 0x01, 0x44, 0xac, 0x00, 0x00, 0x00, 0x00, 0x00};
        auto cr2 = crc32(0, test, sizeof(test));
#endif
        
        uint8_t segmentCount = 0;
        for (const int packetSize : packetSizes) {
            segmentCount += packetSize / 255 +1;
        }
        //uint8_t segmentCount = size / 255 + 1;
        //uint8_t lastSegmentSize = size % 255;
        
        uint16_t offset = kMaxPageHeaderSize - 27 - segmentCount;
        char *header = data.data() + offset;
        
        header[0] = 'O';
        header[1] = 'g';
        header[2] = 'g';
        header[3] = 'S';
        header[4] = 0;
        header[5] = headerType;
        *((uint64_t *)&header[6]) = granule;
        *((uint32_t *)&header[14]) = 0x6EA8914A; // bitstream serial
        *((uint32_t *)&header[18]) = sequence;
        uint32_t checksum = 0;
        *((uint32_t *)&header[22]) = checksum;
        
        header[26] = segmentCount;
        int i = 27;
        for (auto packetSize : packetSizes) {
            bool end = false;
            while(!end) {
                if (packetSize >= 255) {
                    header[i] = 255;
                    packetSize -= 255;
                } else {
                    header[i] = packetSize;
                    end = true;
                }
                ++i;
            }
        }
        
        checksum = crc32(0, reinterpret_cast<uint8_t *>(header), data.size() - offset);
        
        *((uint32_t *)&header[22]) = checksum;

        return offset;
    }
    
    static float *createSilence(size_t size)
    {
        float *silence = new float[size];
        //memset(silence, 0, size * sizeof(float));
        for (int i = 0 ; i < size ; ++i) {
            // real zeros would encode too short, break continuity 
            silence[i] = 0.0000000000001f * rand();
        }

        return silence;
    }

    /*void method clearBuffer()
    {
        // actually, this was a bad idea, since ogg 'remembers' previous music data
        _buffering = true;
        _preBuffer.reset();
    }*/

    void method streamSilence()
    {        
        // we must send some silence, otherwise chrome will hold to it's content
        // from previous song after song switch

        static float *silence = createSilence(4410);

        writeSamplesFromIo(silence, 4410);

        // must fill in buffer for real (invalid) emptiness
        // 8192 is almost ok but contains a bit of lag
        /*static vector<char> emptiness(1500, 0);
        for (auto &client : _clients) {
            if (client->state() == StreamClient::State::Stream) {
                client->write(emptiness,0);
            }
        }*/
    }

    /*void method streamSilence()
    {
        static float phase = 0;
        float silence[4410];
        for (int k = 0 ; k < 2205 ; ++k) {
            silence[2*k] = silence[2*k+1] = sin(phase) * 0.7f;
            phase += 2* M_PI * 440 /44100.0f; 
        }

        writeSamplesFromIo(silence, 4410);
    }*/

    void method writeSamplesFromIo(const float *buf, size_t bufSize)
    {
        if (bufSize == 0) {
            return;
        }
        
        // don't discard, we still need to prebuffer!
        //if (_clients.empty()) {
            // discard this card...
        //    return;
        //}
        
        static int past = 0;
        //if ((++past) < 5) {
            float **channelBuf = vorbis_analysis_buffer(&_dsp, bufSize/2);

            //static float phase = 0;
            for (int i = 0 ; i < bufSize/2 ; ++i) {
                channelBuf[0][i] = buf[2*i];
                channelBuf[1][i] = buf[2*i+1];

            /*    if (_buffering) {
                    channelBuf[0][i] = channelBuf[1][i] = sin(phase) * 0.7f;
                    phase += 2* M_PI * 440 /44100.0f; 
                }*/
            }
            vorbis_analysis_wrote(&_dsp, bufSize/2);
        //}
        int ret;
        //int convertedBytes = 0;

        // one unit is approx. 50ms
        //const int kBufferCount = 30;
        static const int kBufferCount = 0;

        shared_ptr<vector<char>> pPage; 
        _preBuffer.set([&pPage](std::vector<std::pair<int,shared_ptr<std::vector<char>>>> &preBuffer){

            pPage.reset();
            if (preBuffer.size() > kBufferCount) {
                auto p = preBuffer.front();
                //pPage = p.second;
                preBuffer.erase(preBuffer.begin());
            }
        });
        if (!pPage) {
            pPage = shared_ptr<vector<char>>(new vector<char>());
        }
        auto &page = *pPage;
        
        //auto &page = _page;

        page.reserve(kMaxPageHeaderSize + bufSize);
        page.resize(kMaxPageHeaderSize);
        
        std::vector<int> packetSizes;
        uint64_t granule;
        
        do {
            ret = vorbis_analysis_blockout(&_dsp, &_block);
            if (ret <= 0) {
                break;
            }
            if (_block.pcm != NULL) {
                ogg_packet op;
                vorbis_analysis(&_block, NULL);
                vorbis_bitrate_addblock(&_block);
                while(vorbis_bitrate_flushpacket(&_dsp, &op)) {
                
                    page.insert(page.end(), op.packet, op.packet + op.bytes);
                    packetSizes.push_back(op.bytes);
                    granule = op.granulepos;
                }
                //  convertedBytes += op.bytes;
                //ogg_packet_clear(&op);
            }
        } while(ret > 0);
        
        if (packetSizes.empty()) {
            return;
        }
        int offset = generateHeader(page, packetSizes, _sequence++, granule, 0);
        
        //std::cout << "input: " << bufSize *sizeof(float) << " output: " << page.size() - kMaxPageHeaderSize << " packets: " << packetSizes.size() << std::endl;
        
        _clients.erase(remove_if(_clients.begin(),_clients.end(),[&](const shared_ptr<StreamClient> &client){
            if (client->state() == StreamClient::State::Discard) {
                if (!client->serveImage()) {
                    auto w = _worker.lock();
                    if (w) {

                        std::cout << "ChromeCast down\n";
                        w->deviceDown(client->ip());
                    }
                }
                return true;
            } else {
                return false;
            }
        }), _clients.end());

        if (_buffering) {
            if (_preBuffer->size() >= kBufferCount) {
                _buffering = false;
                for (auto &client : _clients) {
                    if (client->state() == StreamClient::State::Stream) {
                        std::vector<std::pair<int,shared_ptr<std::vector<char>>>> preBuffer = _preBuffer;
                        for (auto &p: preBuffer) {
                            client->write(*p.second,p.first);
                        }
                    }
                }
            }
        }
        if (!_buffering) {
            for (auto &client : _clients) {
                if (client->state() == StreamClient::State::Stream) {
                    client->write(page,offset);
                }
            }
        } else {
            //std::cout << "still buffering..." << _preBuffer->size() << std::endl;
        }

        _preBuffer.set([offset,pPage](std::vector<std::pair<int,shared_ptr<std::vector<char>>>> &preBuffer){
            preBuffer.push_back(std::make_pair(offset, pPage));
        });
    }

    void method cancelStreaming()
    {
        // we must cancel streaming so that there is no residual garbage noise when playing again...
        _clients.erase(remove_if(_clients.begin(), _clients.end(), [](const shared_ptr<StreamClient> &client){
            if (client->serveImage()) {
                return false;
            }
            return true;
        }), _clients.end());
    }
}
