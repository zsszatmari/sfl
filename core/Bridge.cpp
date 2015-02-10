#include <iostream>
#include "Bridge.h"
#include "IoService.h"
#include "json.h"

namespace Gear
{
	static bool operator<(Bridge::Privilege lhs, Bridge::Privilege rhs)
	{
		return static_cast<int>(lhs) < static_cast<int>(rhs);
	}

#define method Bridge::

	void method installPost(const string &verb, const function<void(const Json::Value &)> &handler, Privilege privilege)
	{
		Io::get().dispatch([=]{
			_posts[verb] = std::make_pair(handler, privilege);
		});
	}

	void method installGet(const string &verb, const function<Json::Value(const Json::Value &)> &handler, Privilege privilege)
	{
		Io::get().dispatch([=]{
			_gets[verb] = std::make_pair(handler, privilege);
		});
	}
	
	void method post(const string &verb, const Json::Value &value, Privilege privilege)
	{
		std::cout << "POST: " << verb << std::endl;

		auto it = _posts.find(verb);
		if (it == _posts.end()) {
			return;
		}

		std::cout << "POSTing almost: " << verb << std::endl;

		auto &p = it->second;
		if (privilege >= p.second) {
			std::cout << "POSTing: " << verb << std::endl;
			p.first(value);
		}
	}

	void method get(const string &verb, const Json::Value &argument, const function<void(const Json::Value &)> &handler, Privilege privilege)
	{
		auto it = _gets.find(verb);
		Json::Value result;
		if (it == _gets.end()) {
			handler(result);
			return;
		}

		auto &p = it->second;

		if (privilege >= p.second) {
			result = p.first(argument);
		}
		handler(result);
	}

	void method subscribe(const string &verb, const Json::Value &argument, const function<void(const Json::Value &)> &result, Privilege privilege)
	{		
		get(verb, argument, result, privilege);
		auto itSubscribe = _subscribes.find(verb);
		if (itSubscribe != _subscribes.end()) {
			auto subs = itSubscribe->second.first;
			Privilege requiredPrivilege = itSubscribe->second.second;
			if (privilege > requiredPrivilege) {
				subs(argument, result);
			}
		}
	}

	void method unsubscribe(const string &verb, const Json::Value &argument)
	{
		auto it = _unsubscribes.find(verb);
		if (it != _unsubscribes.end()) {
			it->second(argument);
		}
	}

	void method installSubscribe(const string &verb, const function<void(const Json::Value &, const function<void(const Json::Value &)>)> &handler, const function<void(const Json::Value &)> &unsubscribeHandler, Privilege privilege)
	{
		Io::get().dispatch([=]{
			_subscribes[verb] = std::make_pair(handler, privilege);
			_unsubscribes[verb] = unsubscribeHandler;
		});
	}
}
