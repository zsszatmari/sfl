#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <memory>
#include <iostream>
#include "boost/asio.hpp"
#include "stdplus.h"

using namespace boost::asio;
using namespace boost::asio::ip;

using MEMORY_NS::shared_ptr;

class SimpleServer final
{
public:
	SimpleServer(const size_t fileSize) :
		_acceptor(_io),
		_acceptSocket(_io),
		_size(fileSize),
		_sendLimit(fileSize)
	{
		unsigned short port(0);
        tcp::endpoint endPoint(tcp::endpoint(tcp::v4(), port));
        _acceptor.open(endPoint.protocol());
        _acceptor.set_option(tcp::acceptor::reuse_address(true));
        _acceptor.bind(endPoint);
        
        _acceptor.listen();
        endPoint = _acceptor.local_endpoint();
        _port = endPoint.port();

        doAccept();
	}

	int port() const
	{
		return _port;
	}

	void setLimit(size_t sendLimit)
	{
		// only kicks on next connect
		_sendLimit = sendLimit;
	}

	void doAccept()
    {
        _acceptor.async_accept(_acceptSocket, [this](boost::system::error_code ec){

        	//_sockets.push_back(_acceptSocket);

        	readRequest(shared_ptr<tcp::socket>(new tcp::socket(std::move(_acceptSocket))), 0, shared_ptr<boost::asio::streambuf>(new boost::asio::streambuf()));

            doAccept();
        });
    }

    void readRequest(const shared_ptr<tcp::socket> &socket, int rangeBegin, const shared_ptr<boost::asio::streambuf> &buffer)
    {
        async_read_until(*socket, *buffer, '\n', [socket,this,rangeBegin,buffer](boost::system::error_code ec, int bytes) mutable {
            if (ec) {
                return;
            }

            std::istream is(buffer.get());
            std::string line;
            std::getline(is, line);

            std::string kRangePrefix = "Range: bytes=";
            if (line.compare(0, kRangePrefix.length(), kRangePrefix) == 0) {
            	std::stringstream ss(line.substr(kRangePrefix.length()));
            	ss >> rangeBegin;
            }
            
            if (line == "\r") {
            	std::string replyHeader = "HTTP/1.1 200 OK\r\nCache-Control: no-cache, no-store, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\nContent-Type:audio/ogg\r\n\r\n";
            	shared_ptr<std::vector<char>> buf(new std::vector<char>(replyHeader.begin(), replyHeader.end()));
            	int size = _sendLimit - rangeBegin;
            	if (size < 0) {
            		size = 0;
            	}

            	//buf->reserve(size);
            	for (int i = rangeBegin ; i < (rangeBegin + size) ; ++i) {
            		buf->push_back((char)i);
            	}

            	async_write(*socket, boost::asio::buffer(*buf), [socket,this,rangeBegin,buf,replyHeader](boost::system::error_code ec, int bytes){
            		//std::cout << "written " << rangeBegin << " to " << (rangeBegin + bytes - replyHeader.size()) << "\n";
            		if (_sendLimit < _size) {
            			// keep lingering around
            			_sockets.push_back(socket);
            		}
            	});
                return;
            }
            readRequest(socket, rangeBegin,buffer);
        });
    }

    void run()
    {
    	_io.run();
    }

private:
	int _port;
	io_service _io;
	boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _acceptSocket;
    std::vector<shared_ptr<tcp::socket>> _sockets; 

    //std::vector<boost::asio::ip::tcp::socket> _sockets;
    size_t _size;
    size_t _sendLimit;
};


#endif