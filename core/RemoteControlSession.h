#ifndef REMOTECONTROLSESSION_H
#define REMOTECONTROLSESSION_H

#include "stdplus.h"
#include <boost/asio.hpp>
#include "json-forwards.h"
#include SHAREDFROMTHIS_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
	class RemoteControlSession final : public MEMORY_NS::enable_shared_from_this<RemoteControlSession>
	{
	public:
		static shared_ptr<RemoteControlSession> create(boost::asio::ip::tcp::socket socket);
        
    private:
        RemoteControlSession(boost::asio::ip::tcp::socket socket);

        void init();
        void readRequest();
        void reply(const std::string &command, const std::string &arg, const Json::Value &value);

        boost::asio::ip::tcp::socket _socket;
        boost::asio::streambuf _buffer;
	};
}


#endif
