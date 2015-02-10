#ifndef REMOTECONTROLSERVER_H
#define REMOTECONTROLSERVER_H

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <boost/asio.hpp>
#include "stdplus.h"
#include SHAREDFROMTHIS_H
using MEMORY_NS::shared_ptr;
using MEMORY_NS::weak_ptr;

namespace Gear
{
	class RemoteControlSession;

	class RemoteControlServer final : public MEMORY_NS::enable_shared_from_this<RemoteControlServer>
	{
	public:
		static shared_ptr<RemoteControlServer> create();
		virtual ~RemoteControlServer();
		uint16_t port() const;

		static const std::string discoverAddress;
		static const std::string discoverQueryPrefix;
		static const std::string discoverAnswerPrefix;
		static const int discoverPort = 1901;
		
	private:
		RemoteControlServer();

		RemoteControlServer &operator=(const RemoteControlServer &); // delete
		RemoteControlServer(const RemoteControlServer &); //delete

		void init();
		void doAccept();
		void doReplyDiscovery();
		
		boost::asio::ip::tcp::acceptor _acceptor;
        boost::asio::ip::tcp::socket _acceptSocket;
        boost::asio::ip::udp::socket _discoverSocket;
        std::vector<char> _discoverBuffer;
        boost::asio::ip::udp::endpoint _discoverEndpoint;
        std::string _discoverReply;
        uint16_t _port;
        std::vector<shared_ptr<RemoteControlSession>> _sessions;
	};
}

#endif