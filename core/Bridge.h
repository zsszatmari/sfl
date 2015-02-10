#ifndef BRIDGE_H
#define BRIDGE_H

#include "stdplus.h"
#include "json-forwards.h"
#include <map>
#include <functional>

namespace Gear
{
	using std::string;
	using std::function;

	class Bridge final
	{
	public:
		enum class Privilege
		{
			RemoteControlParty = 1000,
			RemoteControl = 2000,
			Full = 3000
		};

		class Subscription;

		// these must be called from the Io thread
		void post(const string &verb, const Json::Value &value, Privilege privilege);
		void get(const string &verb, const Json::Value &argument, const function<void(const Json::Value &)> &result, Privilege privilege);
		void subscribe(const string &verb, const Json::Value &argument, const function<void(const Json::Value &)> &result, Privilege privilege);
		void unsubscribe(const string &verb, const Json::Value &argument);

		// installs can be called from any thread
		void installPost(const string &verb, const function<void(const Json::Value &)> &handler, Privilege privilege);
		// returns a function which should called when there is an update
		// this provides sync getters for fast retrievals. there should be an another version for async getters
		void installGet(const string &verb, const function<Json::Value(const Json::Value &)> &handler, Privilege privilege);
		void installSubscribe(const string &verb, const function<void(const Json::Value &, const function<void(const Json::Value &)>)> &handler, const function<void(const Json::Value &)> &unsubscribeHandler, Privilege privilege);
		
	private:
		std::map<string, std::pair<function<void(const Json::Value &)>,Privilege>> _posts;
		std::map<string, std::pair<function<void(const Json::Value &, const function<void(const Json::Value &)>)>,Privilege>> _subscribes;
		std::map<string, function<void(const Json::Value &)>> _unsubscribes;
		std::map<string, std::pair<function<Json::Value(const Json::Value &)>,Privilege>> _gets;
	};
}


#endif
