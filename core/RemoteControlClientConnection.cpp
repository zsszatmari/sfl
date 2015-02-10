#include "RemoteControlClientConnection.h"
#include "IoService.h"

namespace Gear
{
#define method RemoteControlClientConnection::

	method RemoteControlClientConnection() :
		_socket(Io::get())
	{
	}

	void method async_connect(boost::asio::ip::tcp::endpoint endpoint, const std::function<void(boost::system::error_code ec)> &f)
	{
		_buffer = MEMORY_NS::shared_ptr<boost::asio::streambuf>(new boost::asio::streambuf());
		_socket.lowest_layer().async_connect(endpoint, f);
	}

	void method async_readline(const std::function<void(const std::string &s)> &f)
	{
		auto buffer = _buffer;
		boost::asio::async_read_until(_socket, *_buffer, '\n', [buffer,f](boost::system::error_code ec, std::size_t length){
			if (!ec) {
				boost::asio::streambuf &buf = *buffer;
                std::istream is(&buf);
                std::string line;
                std::getline(is, line);

                f(line);
            }
		});
	}	

	void method async_write(const std::string &s, const std::function<void(boost::system::error_code)> &f)
	{
		MEMORY_NS::shared_ptr<std::string> wrapped(new std::string(s));
		boost::asio::async_write(_socket, boost::asio::buffer(*wrapped), [f, wrapped](boost::system::error_code ec, int sent){
			f(ec);
		});
	}

}