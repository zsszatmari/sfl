#ifdef __APPLE__
#include <dispatch/dispatch.h>
#endif
#include "RemoteDb.h"
#include "GearUtility.h"
#include "IApp.h"
#include "RemoteControl.h"
#include CONDITION_VARIABLE_H

namespace Gear
{
#define method RemoteDb::

	method RemoteDb()
	{
	}

	shared_ptr<IDb> method instance()
	{
		static shared_ptr<IDb> insta(new RemoteDb());
		return insta;
	}	

	// problem is that we don't want to be synchronous... because it would lead to hang in case there is no response:



	std::string method registerView(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const std::function<void(const std::function<void()> &)> &viewQueue, const std::function<void(ClientDb::IView::Event,size_t,size_t)> &callback)
	{
		Json::Value args;
		auto handle = random_string(12);
		args["handle"] = handle;
		args["entity"] = entity;
		args["orderBy"] = serialize(orderBy);
		args["predicate"] = serialize(predicate);
		IApp::instance()->remoteControl()->subscribe("view", args, [callback,viewQueue](const Json::Value &result){

            viewQueue([callback,result]{
                callback(static_cast<ClientDb::IView::Event>(result.get("event",0).asInt()), result.get("offset",0).asInt(), result.get("size",0).asInt());
            });
		});

		_views[handle] = args;

		return handle;
	}

	void method unregisterView(const std::string &handle)
	{
		auto it = _views.find(handle);
		if (it != _views.end()) {

			IApp::instance()->remoteControl()->unsubscribe("view", it->second);
			_views.erase(it);
		}
	}

	static Json::Value getWithTimeout(const std::string &verb, const Json::Value &args)
	{
		// warning: this would cause hang if called from the io thread! 
		// must be called from main thread
#ifdef __APPLE__
		assert(dispatch_get_current_queue() == dispatch_get_main_queue());
#endif

		shared_ptr<THREAD_NS::condition_variable> cond(new THREAD_NS::condition_variable());
		shared_ptr<bool> finished(new bool(false));
		shared_ptr<Json::Value> ret(new Json::Value());
		shared_ptr<THREAD_NS::mutex> m(new THREAD_NS::mutex());

		IApp::instance()->remoteControl()->get(verb, args, [finished,ret,m,cond](const Json::Value &result){
			THREAD_NS::lock_guard<THREAD_NS::mutex> l(*m);
			*ret = result;
			*finished = true;
			cond->notify_all();
		});


		THREAD_NS::unique_lock<THREAD_NS::mutex> l(*m);
		cond->wait_for(l, CHRONO_NS::milliseconds(500), [&]{return *finished;});

		return *ret;
	}

	SongData method viewAt(const std::string &handle, size_t index) const
	{
		Json::Value args(Json::objectValue);
		args["handle"] = handle;
		args["index"] = (uint32_t)index;
		auto result = getWithTimeout("viewAt", args);
	
		return result;
	}

	size_t method viewSize(const std::string &handle) const
	{
		Json::Value args(Json::objectValue);
		args["handle"] = handle;
		auto result = getWithTimeout("viewSize", args);
	
		if (result.isNumeric()) {
			return result.asInt();
		} else {
			return 0;
		}
	}

	void method viewSetVisibleRange(const std::string &handle, size_t offset, size_t size)
	{
		Json::Value arg(Json::objectValue);
		arg["handle"] = handle;
		arg["offset"] = (uint32_t)offset;
		arg["size"] = (uint32_t)size;
		IApp::instance()->remoteControl()->post("viewSetVisibleRange", arg); 
	}

	std::vector<Json::Value> method fetchSync(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, size_t limit, size_t offset, bool waitForChanges)
	{
		Json::Value args(Json::objectValue);
		args["entity"] = entity;
		args["predicate"] = serialize(predicate);
		args["orderBy"] = serialize(orderBy);
		args["limit"] = (uint32_t)limit;
		args["offset"] = (uint32_t)offset;
		args["waitForChanges"] = waitForChanges;
		auto result = getWithTimeout("fetch", args);
	
		std::vector<Json::Value> ret;
		if (result.isArray()) {
			for (auto i = 0 ; i < result.size() ; ++i) {
				ret.push_back(result[i]);
			}
		}
 
		return ret;
	}

	std::vector<Json::Value> method fetchGroupings(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy,
												const std::vector<std::string> &fetchFields, const std::vector<std::string> &groupBy) 
	{
		Json::Value args(Json::objectValue);
		args["entity"] = entity;
		args["predicate"] = serialize(predicate);
		args["orderBy"] = serialize(orderBy);
		args["fields"] = serialize(fetchFields);
		args["groupBy"] = serialize(groupBy);
		auto result = getWithTimeout("fetchGroups", args);
	
		std::vector<Json::Value> ret;
		if (result.isArray()) {
			for (auto i = 0 ; i < result.size() ; ++i) {
				ret.push_back(result[i]);
			}
		}
 
		return ret;
	}

	std::vector<std::string> method fetchSync(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const std::string &field)
	{
		Json::Value args(Json::objectValue);
		args["entity"] = entity;
		args["predicate"] = serialize(predicate);
		args["orderBy"] = serialize(orderBy);
		args["field"] = field;
		auto result = getWithTimeout("fetchField", args);
	
		std::vector<std::string> ret;
		if (result.isArray()) {
			for (auto i = 0 ; i < result.size() ; ++i) {
				ret.push_back(result[i].asString());
			}
		}
 
		return ret;
	}

	uint32_t method count(const std::string &entity, const ClientDb::Predicate &predicate)
	{
		Json::Value args(Json::objectValue);
		args["entity"] = entity;
		args["predicate"] = serialize(predicate);
		auto result = getWithTimeout("count", args);
	
		if (result.isNumeric()) {
			return result.asInt();
		} else {
			return 0;
		}
	}

	Json::Value method serialize(const ClientDb::Predicate &predicate)
	{
		Json::Value ret;
		ret["key"] = predicate.key();
		ret["value"] = predicate.value();
		ret["op"] = static_cast<int>(predicate.op());
		
		Json::Value subs(Json::arrayValue);
		for (const auto &sub : predicate.predicates()) {
			subs.append(serialize(sub));
		}
		ret["subs"] = subs;

		return ret;
	}

	ClientDb::Predicate method deserializePredicate(const Json::Value &json)
	{
		if (!json.isObject()) {
			return ClientDb::Predicate();
		}
		auto subs = json.get("subs", Json::arrayValue);
		std::vector<ClientDb::Predicate> subpredicates;
		for (int i = 0 ; i < subs.size() ; ++i) {
			subpredicates.push_back(deserializePredicate(subs[i]));
		}

		return ClientDb::Predicate(static_cast<ClientDb::Predicate::Operator>(json.get("op",0).asInt()), json.get("key","").asString(), json.get("value",""), subpredicates);
	}

	Json::Value method serialize(const ClientDb::SortDescriptor &sortDescriptor)
	{
		Json::Value ret(Json::objectValue);
		Json::Value keys(Json::arrayValue);

		for (const auto &key : sortDescriptor.orderBy()) {
			keys.append(key);
		}
		ret["orderBy"] = keys;
		ret["ascending"] = sortDescriptor.ascending();
		return ret;
	}

	ClientDb::SortDescriptor method deserializeSortDescriptor(const Json::Value &json)
	{
		if (!json.isObject()) {
			return ClientDb::SortDescriptor();
		}
		std::vector<std::string> orderBy;
		auto keys = json.get("orderBy",Json::arrayValue);
		for (int i = 0 ; i < keys.size() ; ++i) {
			orderBy.push_back(keys[i].asString());
		}
		return ClientDb::SortDescriptor(orderBy, json.get("ascending",true).asBool());
	}

	Json::Value method serialize(const std::vector<std::string> &strings)
	{
		Json::Value ret(Json::arrayValue);
		for (auto &str : strings) {
			ret.append(str);
		}
		return ret;
	}

	std::vector<std::string> method deserializeStrings(const Json::Value &json)
	{
		std::vector<std::string> ret;
		if (!json.isArray()) {
			return ret;
		}
		for (int i = 0 ; i < json.size() ; ++i) {
			ret.push_back(json[i].asString());
		}
		return ret;
	}

	size_t method indexOf(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const Json::Value &item)
	{
		return 0;
		// nada
	}

	void method remove(const std::string &entity, const ClientDb::Predicate &predicate)
	{
		// nyet
	}

	void method remove(const std::string &entity, const Json::Value &object)
	{
		// no
	}

	int32_t method max(const std::string &entity, const ClientDb::Predicate &predicate, const std::string &field)
	{
		return 0;
		// nichts
	}

}