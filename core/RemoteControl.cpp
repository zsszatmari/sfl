#include <iostream>
#include "RemoteControl.h"
#include "RemoteControlServer.h"
#include "IoService.h"
#include "IApp.h"
#include "PlayerSwitch.h"
#include "PlayerRemote.h"
#include "json.h"
#include "SessionManager.h"

namespace Gear
{

#define method RemoteControl::

	shared_ptr<RemoteControl> method create()
	{
		shared_ptr<RemoteControl> ret(new RemoteControl());
		ret->init();
		return ret;
	}

	method RemoteControl() :
		_discoverQuery(RemoteControlServer::discoverQueryPrefix + "\n" + "Machina"),
		_discoverSocket(Io::get())
	{
        _controlling = false;
		_discoverSocket.open(boost::asio::ip::udp::v4());
        _discoverSocket.set_option(boost::asio::socket_base::broadcast(true));
	}

	void method discover(const shared_ptr<DiscoverDelegate> &delegate)
	{
		_discoverDelegate = delegate;

		boost::asio::ip::udp::endpoint destination;
        destination.address(boost::asio::ip::address::from_string(RemoteControlServer::discoverAddress));
        //destination.address(boost::asio::ip::address::from_string("255.255.255.255"));
        destination.port(RemoteControlServer::discoverPort);

        // first start waiting for reply, then send
       	waitDiscoverReply();

        _discoverSocket.async_send_to(boost::asio::buffer(_discoverQuery), destination, [](boost::system::error_code ec, int bytes){
        	std::cout << "sent discover query " << ec << " bytes: " << bytes << "\n";
        });
	}

	void method waitDiscoverReply()
	{
		std::cout << "searching for controllable...\n";

        _discoverReply.resize(512);
        weak_ptr<RemoteControl> wself = shared_from_this();
		_discoverSocket.async_receive_from(boost::asio::buffer(_discoverReply), _discoverReplier, [wself,this](boost::system::error_code ec, std::size_t length){
        	std::cout << "got controllable reply... (almost) " << ec << "\n";

        	auto self = wself.lock();
        	if (!self) {
        		return;
        	}

        	if (!ec) {
                std::cout << "got controllable reply...\n";
                
        		auto delegate = _discoverDelegate.lock();
        		if (delegate) {
        			_discoverReply.resize(length);
        			std::string reply(_discoverReply.begin(), _discoverReply.end());
        			std::stringstream ss(reply);

        			std::string replyBody;
        			std::string machine;
        			std::string portStr;
        			getline(ss, replyBody);
        			if (replyBody == RemoteControlServer::discoverAnswerPrefix) {
        				getline(ss, machine);
        				getline(ss, portStr);
        				if (!portStr.empty()) {
        					std::stringstream ss(portStr);
        					int port;
        					ss >> port; 

        					auto ip = _discoverReplier.address().to_string();
        					std::cout << "found controllable at " << ip << " : " << port << std::endl; 

	        				delegate->foundRemoteControllable(machine, ip, port);
	        			}
        			}
        		}
        	} 
        	waitDiscoverReply();
        });
	}

	void method init()
	{
	}	

	void method start(const string &ip, int port)
	{
        //_buffer.consume(_buffer.size());
        _key.clear();

		boost::asio::ip::tcp::endpoint destination;
        destination.port(port);
        destination.address(boost::asio::ip::address::from_string(ip));

        weak_ptr<RemoteControl> wself = shared_from_this();
        _controlling = true;

        _connection.async_connect(destination, [wself,this](boost::system::error_code ec){
        	auto self = wself.lock();
        	if (!self) {
        		return;
        	}

        	if (ec) {
        		stop();
        		return;
        	}

            waitForData();

			IApp::instance()->playerSwitch()->push(PlayerRemote::create(self));
            subscribe("categories", Json::Value(), [this](const Json::Value &arg){
                
                {
                    std::lock_guard<std::mutex> l(_categoriesMutex);
                    _playlistCategories = PlaylistCategory::deserialize(arg);
                }
                IApp::instance()->sessionManager()->recomputeCategories();
            });
        });
	}

    void method waitForData()
    {
        auto self = shared_from_this();
        _connection.async_readline([self,this](const std::string &line) {
            if (_key.empty()) {
                _key = line;
            } else {
                Json::Reader parser;
                Json::Value value;
                parser.parse(line, value);

                auto it = _subscriptions.find(_key);
                if (it != _subscriptions.end()) {
                    it->second(value);
                }
                it = _pendingGets.find(_key);
                if (it != _pendingGets.end()) {
                    it->second(value);
                    _pendingGets.erase(it);
                }

                _key.clear();
            }
            waitForData();
        });
    }

	void method stop()
	{
        _controlling = false;
        _subscriptions.clear();
		IApp::instance()->playerSwitch()->pop();
	}

    bool method controlling() const
    {
        return _controlling;
    }

    static std::string verbAndParam(const string &verb, const Json::Value &arg)
    {
        std::string str;
        {
            std::stringstream ss;
            Json::FastWriter writer;
            ss << verb << " " << writer.write(arg);
            str = ss.str();
            if (str[str.size()-1] == '\n') {
                str = str.substr(0, str.size() -1);
            }
        }
        return str;
    }

	void method post(const string &verb, const Json::Value &arg)
	{
		auto s = std::string("P") + verbAndParam(verb,arg) + "\n";
        auto self = shared_from_this();

		_connection.async_write(s, [self,this](boost::system::error_code ec){
            if (ec) {
                stop();
            }
			// we can drop string now
		});
	}

    void method subscribe(const string &verb, const Json::Value &arg, const std::function<void(const Json::Value &)> &callback)
    {
        auto str = verbAndParam(verb, arg);
        
        auto self = shared_from_this();
        Io::get().dispatch([self,this,str,callback]{
            _subscriptions[str] = callback;
        });
        std::string s("S" + str + '\n');
        

        _connection.async_write(s, [self,this](boost::system::error_code ec){
            if (ec) {
                stop();
            }
        });
    }

    void method unsubscribe(const string &verb, const Json::Value &arg)
    {
        auto s = std::string("U") + verbAndParam(verb,arg) + "\n";
        auto self = shared_from_this();

        _connection.async_write(s, [self,this](boost::system::error_code ec){
            if (ec) {
                stop();
            }
        });
    }

    void method get(const string &verb, const Json::Value &arg, const std::function<void(const Json::Value &)> &callback)
    {
        auto str = verbAndParam(verb, arg);
        auto s = std::string("G") + verbAndParam(verb,arg) + "\n";

        auto self = shared_from_this();
        Io::get().dispatch([self,this,str,callback]{
            _pendingGets.insert(std::make_pair(str, callback));
        });

        _connection.async_write(s, [self,this](boost::system::error_code ec){
            if (ec) {
                stop();
            }
        });
    }

    std::vector<PlaylistCategory> method playlistCategories() const
    {
        std::lock_guard<std::mutex> l(_categoriesMutex);
        return _playlistCategories;
    }
}