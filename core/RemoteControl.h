#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include "boost/asio.hpp"
#include "stdplus.h"
#include "json-forwards.h"
#include SHAREDFROMTHIS_H
#include ATOMIC_H
using MEMORY_NS::shared_ptr;
using MEMORY_NS::weak_ptr;
#include MUTEX_H
#include "PlaylistCategory.h"
#include "RemoteControlClientConnection.h"

namespace Gear
{
	class RemoteControl final : public MEMORY_NS::enable_shared_from_this<RemoteControl>
	{
	public:
		static shared_ptr<RemoteControl> create();

		class DiscoverDelegate
		{
		public:
			virtual void foundRemoteControllable(const std::string &name, const std::string &ip, int port) = 0;
			virtual ~DiscoverDelegate() {}
		};

		void discover(const shared_ptr<DiscoverDelegate> &delegate);
		void start(const std::string &ip, int port);
		void stop();
		bool controlling() const;
		void post(const std::string &verb, const Json::Value &arg);
		void get(const string &verb, const Json::Value &arg, const std::function<void(const Json::Value &)> &callback);
	    void subscribe(const string &verb, const Json::Value &arg, const std::function<void(const Json::Value &)> &callback);
	    void unsubscribe(const string &verb, const Json::Value &arg);

		std::vector<PlaylistCategory> playlistCategories() const;

	private:
		RemoteControl();

		void init();
		void waitDiscoverReply();
	    void waitForData();


		ATOMIC_NS::atomic_bool _controlling;

		RemoteControlClientConnection _connection;
		boost::asio::ip::udp::socket _discoverSocket;
		boost::asio::ip::udp::endpoint _discoverReplier;
		const std::string _discoverQuery;
		std::vector<char> _discoverReply;
		weak_ptr<DiscoverDelegate> _discoverDelegate;

		std::string _key;
		std::map<std::string, std::function<void(Json::Value &)>> _subscriptions;
		std::multimap<std::string, std::function<void(Json::Value &)>> _pendingGets;

		mutable std::mutex _categoriesMutex;
		std::vector<PlaylistCategory> _playlistCategories;
	};
}

#endif