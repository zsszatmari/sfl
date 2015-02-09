#ifndef REMOTECONTROLCLCONN_H
#define REMOTECONTROLCLCONN_H

#include "boost/asio.hpp"
#include "stdplus.h"
#include MEMORY_H

namespace Gear
{
	class RemoteControlClientConnection final
	{
	public:
		RemoteControlClientConnection();
		void async_connect(boost::asio::ip::tcp::endpoint, const std::function<void(boost::system::error_code)> &f);
		void async_readline(const std::function<void(const std::string &s)> &f);
		void async_write(const std::string &s, const std::function<void(boost::system::error_code)> &f);

	private:
		boost::asio::ip::tcp::socket _socket;
		MEMORY_NS::shared_ptr<boost::asio::streambuf> _buffer;
	};
}

#endif