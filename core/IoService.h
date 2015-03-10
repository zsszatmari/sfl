#ifndef GEAR_IO_H
#define GEAR_IO_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "stdplus.h"
#include THREAD_H

namespace Gear
{
	class Io
	{
	public:
		static boost::asio::io_service & get();
		static bool isCurrent();
		static void terminate();

	private:
		Io();

		static Io &instance();

		Io(const Io &); // delete
		Io &operator=(const Io &); // delete

		boost::asio::io_service _io;
		boost::asio::ssl::context _context;
		THREAD_NS::thread::id _threadId;
	};
}

#endif
