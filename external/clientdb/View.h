#ifndef VIEW_H
#define VIEW_H

#include <string>
#include <map>
#include <set>
#include <functional>
#include "stdplus.h"
#include MEMORY_H
#include SHAREDFROMTHIS_H
#include ATOMIC_H
#include "jsoncpp/json-forwards.h"
#include "Index.h"
#include "Predicate.h"
using MEMORY_NS::shared_ptr;
using MEMORY_NS::weak_ptr;

namespace ClientDb
{
	class Db;

	class View : public MEMORY_NS::enable_shared_from_this<View>
	{
	public:
		enum class Event
		{
			Inserted = 0,
			Deleted = 1,
			Updated = 2,
			Reload = 3
		};

		// by default callbacks are executed on the db delegate queue but one can easily forward them to app main thread if desired
		// also the methods are thread safe, and non-blocking

		Json::Value at(size_t index) const;
		void setVisibleRange(size_t offset, size_t size);
		size_t size() const;

		void fetchRange(size_t pos, size_t size);
		
	private:
		View(const std::string &entity, const std::function<void(const std::function<void()> &)> &viewQueue, const std::function<void(const shared_ptr<View> &,Event,size_t,size_t)> &callback, const shared_ptr<Db> &db, const SortDescriptor &orderBy, const Predicate &predicate, size_t prefetchSize = 32);

		View(const View &); // delete
		View &operator=(const View &); // delete

		// called from ClientDb
		void inserted(const Json::Value &object);
		void updated(const Json::Value &oldObject, const Json::Value &object);
		void deleted(const Json::Value &oldObject);
		void deleted(const Predicate &predicate);
		
		//int findObject(const Json::Value &object);
		void queueSync(const std::function<void()> &f) const;

		mutable std::map<size_t, Json::Value> _fetchedValues;
		int _size;
		//mutable mutex _m;

		ATOMIC_NS::atomic_int _visibleOffset;
		ATOMIC_NS::atomic_int _visibleSize;

		const int _prefetchSize;
		const std::string _entity;
		const std::function<void(const std::function<void()> &)> _queue;
		const std::function<void(const shared_ptr<View> &,Event,size_t,size_t)> _callback;
		const weak_ptr<Db> _db;
		const SortDescriptor _orderBy;
		const Predicate _predicate;

		friend class Db;
	};
}

#endif