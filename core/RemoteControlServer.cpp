#include <string>
#include <iostream>
#include "RemoteControlServer.h"
#include "RemoteControlSession.h" 
#include "IoService.h"

using namespace boost::asio::ip;

namespace Gear
{
    const std::string RemoteControlServer::discoverAddress = "255.255.255.255";
    const std::string RemoteControlServer::discoverQueryPrefix = "GearRC?";
    const std::string RemoteControlServer::discoverAnswerPrefix = "GearRC!";

#define method RemoteControlServer::

	shared_ptr<RemoteControlServer> method create()
	{
		shared_ptr<RemoteControlServer> ret(new RemoteControlServer());
		ret->init();
		return ret;
	}

	void method init()
	{
		unsigned short port(0);
        tcp::endpoint endPoint(tcp::endpoint(tcp::v4(), port));
        _acceptor.open(endPoint.protocol());
        _acceptor.set_option(tcp::acceptor::reuse_address(true));
        _acceptor.bind(endPoint);
        
        _acceptor.listen();
        endPoint = _acceptor.local_endpoint();
        _port = endPoint.port();

        std::cout << "listening for Remote control (unencrypted!) on port " << _port << std::endl;

        doAccept();
        doReplyDiscovery();
	}

    uint16_t method port() const
    {
        return _port;
    }

    method ~RemoteControlServer()
    {
    }   

	void method doAccept()
    {
    	weak_ptr<RemoteControlServer> wself = shared_from_this();
        _acceptor.async_accept(_acceptSocket, [wself,this](boost::system::error_code ec){

            auto self = wself.lock();
            if (!self) {
                return;
            }
            std::cout << "accepting rc\n";

            if (!ec) {
                /*
                boost::error_code setEc;
                {
                    boost::asio::ip::tcp::no_delay option(true);
                    socket.set_option(option, setEc);
                }
                boost::asio::ip::tcp::no_delay option;
                bool isSet = socket.get_option();
                */

                _sessions.push_back(RemoteControlSession::create(std::move(_acceptSocket)));
            }
            doAccept();
        });
    }

    void method doReplyDiscovery()
    {
        weak_ptr<RemoteControlServer> wself = shared_from_this();
        _discoverBuffer.resize(512);
        _discoverSocket.async_receive_from(boost::asio::buffer(_discoverBuffer), _discoverEndpoint, [wself,this](boost::system::error_code ec, int bytes){
            
            std::cout << "got discovery..." << ec << "\n";

            auto self = wself.lock();
            if (!self) {
                return;
            }
            if (!ec) {
                _discoverBuffer.resize(bytes);
                std::string reply(_discoverBuffer.begin(), _discoverBuffer.end());
                std::cout << "got discovery: " << reply << "\n";

                if (reply.compare(0,discoverQueryPrefix.length(), discoverQueryPrefix) == 0) {
                    std::string machineName = "servomachina";
                    std::stringstream ss;
                    ss << discoverAnswerPrefix << "\n" << machineName << "\n" << _port;
                    _discoverReply = ss.str();
                    
                    _discoverSocket.async_send_to(boost::asio::buffer(_discoverReply), _discoverEndpoint,[](boost::system::error_code ec,int bytes){
                        // sent off reply, now we are uninterested
                    });
                }
            }
            doReplyDiscovery();
        });
    }

	method RemoteControlServer() :
		_acceptor(Io::get()),
		_acceptSocket(Io::get()),
        //TODO: port might be used in another app
        _discoverSocket(Io::get())
	{
        _discoverSocket.open(boost::asio::ip::udp::v4());
        _discoverSocket.set_option(decltype(_discoverSocket)::reuse_address(true));

        boost::asio::ip::udp::endpoint ep(boost::asio::ip::udp::v4(), discoverPort);
        _discoverSocket.bind(ep);
	}	
}