#include <iostream>
#include "RemoteControlSession.h"
#include "Bridge.h"
#include "json.h"
#include "IApp.h"

namespace Gear
{
#define method RemoteControlSession::

	shared_ptr<RemoteControlSession> method create(boost::asio::ip::tcp::socket socket)
	{
		shared_ptr<RemoteControlSession> ret(new RemoteControlSession(std::move(socket)));
		ret->init();
		return ret;
	}

	method RemoteControlSession(boost::asio::ip::tcp::socket socket) :
		_socket(std::move(socket))
	{
	}

	void method init()
	{
		readRequest();
	}

	void method readRequest()
	{
		//std::cout << "reading request...\n";

		weak_ptr<RemoteControlSession> wself = shared_from_this();
		async_read_until(_socket, _buffer, '\n', [wself](boost::system::error_code ec, int bytes){
			auto self = wself.lock();
			if (!self) {
				return;
			}

			auto privilege = Bridge::Privilege::RemoteControl;

			std::istream is(&self->_buffer);
            std::string line;
            std::getline(is, line);

            auto spacePos = line.find(' ');

            // first character is the method
            // example: Gcategories {}\n
			if (!line.empty()) {
				auto command = line.substr(1, spacePos-1);
				Json::Value arg;
				std::string argStr;
				if (spacePos != std::string::npos && spacePos > 0) {
					argStr = line.substr(spacePos+1);
					Json::Reader parser;
					parser.parse(argStr, arg);
				}

				std::cout << "request... '" << line[0] << "' " << command << "\n";
                try {
                    switch(line[0]) {
                        case 'P':
                            // post
                            IApp::instance()->bridge()->post(command, arg, privilege);
                            break;
                        case 'G':
                            // get
                            IApp::instance()->bridge()->get(command, arg, [wself,command,argStr](const Json::Value &value){
                                auto self = wself.lock();
                                if (self) {
                                    self->reply(command, argStr, value);
                                    
                                }
                            }, privilege);
                            break;
                        case 'S':
                            // subscribe
                            IApp::instance()->bridge()->subscribe(command, arg, [command, argStr, wself](const Json::Value &result){
                                auto self = wself.lock();
                                if (self) {
                                    self->reply(command, argStr, result);
                                }
                            }, privilege);
                            break;
                        case 'U':
                            // unsubscribe
                            IApp::instance()->bridge()->unsubscribe(command, arg);
                            break;
                    }
                } catch (std::exception &e) {
                    std::cout << "error in remote request " << e.what() << " '" << line << "'\n";
                    self->_socket.close();
                    return;
                }
			}
            self->readRequest();
		});
	}

	void method reply(const std::string &command, const std::string &arg, const Json::Value &value)
	{
		Json::FastWriter writer;
		std::stringstream ss;
		ss << command << " " << arg << "\n" << writer.write(value);
		auto str = ss.str();
		if (str[str.length() -1] != '\n') {
			str += '\n';
		}
		//std::cout << "replying: " << ss.str() << std::endl;

		shared_ptr<std::string> s(new std::string(str));
		async_write(_socket, boost::asio::buffer(*s), [s](boost::system::error_code ec, int len){
		});
	}
}