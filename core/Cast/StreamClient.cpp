//
//  StreamClient.cpp
//  Cast
//
//  Created by Zsolt Szatmari on 06/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include "StreamClient.h"
#include "Streamer.h"
#include "BitmapImage.h"
#include "NamedImage.h"
#include "IApp.h"
#include "IPlayer.h"
#include "stdplus.h"
#include "PromisedImage.h"
#include "IPlaybackWorker.h"

namespace Cast
{
#define method StreamClient::
    
    using namespace boost::asio;
    using namespace Gear;
    using namespace Gui;
    using std::vector;
    
    shared_ptr<StreamClient> method create(boost::asio::ip::tcp::socket socket, const vector<char> &header)
    {
        shared_ptr<StreamClient> ret(new StreamClient(std::move(socket), header));
        ret->readRequest();
        return ret;
    }
    
    method StreamClient(boost::asio::ip::tcp::socket socket, const vector<char> &header) :
        _socket(std::move(socket)),
        _state(State::Open),
        _writing(false),
        _header(header),
        _serveImage(false),
        _firstLine(true),
        _ip(_socket.lowest_layer().remote_endpoint().address().to_string())
    {  
    }

    string method ip() const
    {
        return _ip;
    }
        
    
    StreamClient::State method state() const
    {
        return _state;
    }
    
    void method readRequest()
    {
        auto self = shared_from_this();
        async_read_until(_socket, _buffer, '\n', [self](boost::system::error_code ec, int bytes){
            if (ec) {
                self->_state = State::Discard;
                return;
            }

            std::istream is(&self->_buffer);
            std::string line;
            std::getline(is, line);

            if (self->_firstLine) {
                self->_firstLine = false;
                if (line.find(".png") != std::string::npos) {
                    self->_serveImage = true;
                }
            }
            
            if (line == "\r") {
                self->beginWrite();
                return;
            }
            self->readRequest();
        });
    }

    bool method serveImage() const
    {
        return _serveImage;
    }
    
    void method beginWrite()
    {        
        if (_serveImage) {
            std::string replyHeader = "HTTP/1.1 200 OK\r\nCache-Control: no-cache, no-store, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\nContent-Type:image/png\r\n\r\n";
            std::vector<char> data(replyHeader.begin(), replyHeader.end());

            auto promise = IApp::instance()->player()->albumArt(512);
            auto image = promise->image();
            auto bitmap = MEMORY_NS::dynamic_pointer_cast<BitmapImage>(image);
            if (bitmap) {
#ifdef DEBUG
                //milliseconds w(5000);
                //sleep_for(w);
#endif

                const auto &bitmapData = bitmap->data();
                data.insert(data.end(), bitmapData.begin(), bitmapData.end());
            } else {
                auto named = MEMORY_NS::dynamic_pointer_cast<NamedImage>(image);
                if (named) {
                    std::stringstream ss;
                    // wrong: don't need noart-cocoa
                    //string imageName = named->imageName();
                    string imageName = "noart";
                    ss << "HTTP/1.1 303 See Other\r\nLocation: http://treasurebox-gear.s3.amazonaws.com/cast/" << imageName << ".png\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
                    auto str = ss.str();
                    data = std::vector<char>(str.begin(), str.end());
                }
            }

            auto self = shared_from_this();
            _state = State::Discard;
            async_write(_socket, boost::asio::buffer(data), [self](boost::system::error_code ec, std::size_t length){
                self->_socket.close();
            });
            return;
        }
    
        /*
         icecast example working which is actually working in chrome:
         < HTTP/1.0 200 OK
         < Server: Icecast 2.3.99.0
         < Date: Sat, 10-May-2014 18:41:27 GMT
         < Content-Type: application/ogg
         < Cache-Control: no-cache
         < icy-br:64
         < ice-audio-info: bitrate=64
         < icy-description:FM3 Buddha Machine simulation example
         < icy-genre:test
         < icy-name:Endless drone - Opus codec example
         < icy-pub:1
         < icy-url:http://opus-codec.org/examples/

         */
        
        std::string replyHeader = "HTTP/1.1 200 OK\r\nCache-Control: no-cache, no-store, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\nContent-Type:audio/ogg\r\n\r\n";
        //std::string replyHeader = "HTTP/1.1 200 OK\r\nCache-Control: no-cache, no-store, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\nContent-Type:application/ogg\r\n\r\n";
        
        
        std::vector<char> data(replyHeader.begin(), replyHeader.end());
        data.insert(data.end(), _header.begin(), _header.end());
        vector<char> empty;
        _header.swap(empty);
        
// this would be faulty, does not take page offset into account
//#define TEST_OUTPUT_FILE "/Users/zsszatmari/tmp/test.ogg"
#ifdef TEST_OUTPUT_FILE
        FILE *f = fopen(TEST_OUTPUT_FILE, "w");
        fwrite(_header->data(), 1, _header->size(), f);
        fclose(f);
#endif
        _state = State::Stream;
        write(data, 0);
    }
    
    void method write(const std::vector<char> &buf, size_t offset)
    {
        if (_state != State::Stream) {
            return;
        }
        
        //std::cout << "writing " << buf.size() << " bytes\n";
     
#ifdef TEST_OUTPUT_FILE
        FILE *f = fopen(TEST_OUTPUT_FILE, "a");
        fwrite(buf.data(), 1, buf.size(), f);
        fclose(f);
#endif
        
        if (_writing) {
            _pendingBuffer.insert(_pendingBuffer.end(), buf.begin()+offset, buf.end());
            return;
        } else {
            _sendingBuffer.clear();
            swap(_pendingBuffer, _sendingBuffer);
            _sendingBuffer.insert(_sendingBuffer.end(), buf.begin()+offset, buf.end());
        }
        
        if (_sendingBuffer.empty()) {
            return;
        }
        
        auto sendingSize = _sendingBuffer.size();
        //std::cout << "bytes in send buffer: " << sendingSize << " this: " << this << std::endl;
        
        _writing = true;
        auto self = shared_from_this();
        async_write(_socket, boost::asio::buffer(_sendingBuffer.data(), _sendingBuffer.size()), [self,sendingSize](boost::system::error_code ec, std::size_t length){
            
            if (ec) {
                self->_state = State::Discard;
                std::cout << "ChromeCast stream connection closed\n";
                return;
            }
            //std::cout << "streamed " << length << " bytes out of " << sendingSize << "\n";
            
            self->_writing = false;
            self->write(std::vector<char>(), 0);
        });
    }
}
