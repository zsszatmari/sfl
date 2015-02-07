#ifndef CLIENTDB_DB_H
#define CLIENTDB_DB_H

#include <functional>
#include "stdplus.h"
#include "Serializer.h"
#include "jsoncpp/json.h"
#include "Index.h"
#include "View.h"
#include "IBackend.h"
#include "SqliteBackend.h"

namespace ClientDb
{
	class Db final : public MEMORY_NS::enable_shared_from_this<Db>
	{
	public:
		static shared_ptr<Db> open(const std::string &filename, const std::vector<Index> &indexes, const std::function<void(const std::function<void()> &)> &delegateQueue, const std::function<void(const shared_ptr<Db> &)> &callback, const shared_ptr<IBackend> &backend = shared_ptr<IBackend>(new SqliteBackend()), Comparator comparator = nullptr);
		// no callback version;
		static shared_ptr<Db> openSync(const std::string &filename, const std::vector<Index> &indexes, const std::function<void(const std::function<void()> &)> &delegateQueue, const shared_ptr<IBackend> &backend = shared_ptr<IBackend>(new SqliteBackend()), Comparator comparator = nullptr);
		static void destroy(const std::string &filename, const shared_ptr<IBackend> &backend = shared_ptr<IBackend>(new SqliteBackend()));

		~Db();
		// fields that are not mentioned in object are not modified (nulled) ! they are left alone
		// oldobjecthandler will be executed synchronosuly (must be non-blocking) and it is the last chance to modify
		void upsert(const std::string &entity, const Json::Value &object, const std::function<void(const Json::Value &, Json::Value &)> &oldObjectHandler = std::function<void(const Json::Value &, Json::Value &)>());
		void remove(const std::string &entity, const Json::Value &object);
		// was deprecated: can't be done correctly
		//void removeOtherThan(const std::string &entity, const Predicate &predicate, const std::string &field, const std::vector<std::string> &keepThese);

		//void findOne(const std::string &entity, const Json::Value &query, const function<void(const Json::Value &)> &result);
		std::vector<Json::Value> fetchSync(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, size_t limit = 0, size_t offset = 0, bool waitForChanges = true);

		shared_ptr<View> view(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const std::function<void(const std::function<void()> &)> &viewQueue, const std::function<void(const shared_ptr<View> &,View::Event,size_t,size_t)> &callback);
		void freeUpMemory();

		// these are the more delicate methods:

		// use this very judiciously. this is supposed to remove a whole playlist where there is exact match for views. superset okay, but no overlaps
		void remove(const std::string &entity, const Predicate &predicate);
		// use this very judiciously, will be slow for larget sets
		int32_t max(const std::string &entity, const Predicate &predicate, const std::string &field);
		uint32_t count(const std::string &entity, const Predicate &predicate, int connectionId);
		size_t indexOf(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const Json::Value &item);
		std::vector<std::string> fetchSync(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const std::string &field);
		std::vector<Json::Value> fetchGroupings(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy,
											const std::vector<std::string> &fetchFields, const std::vector<std::string> &groupBy);

		std::vector<std::string> remove(const std::string &entity, const ClientDb::Predicate &predicate, 
					const std::string &field, const std::vector<std::string> &values);

	private:
		Db(const std::string &filename, const std::map<std::string,std::vector<Index>> &indexes, const std::function<void(const std::function<void()> &)> &delegateQueue, const shared_ptr<IBackend> &backend, Comparator comparator = nullptr);

		void size(const std::string &entity, const Predicate &predicate, const std::function<void(size_t)> &immediateResult);
		Predicate predicateForPrimaryIndex(const std::string &entity, const Json::Value &object) const;

		void enumerateViews(const std::function<void(const shared_ptr<View> &)> &f);
		std::function<void(const std::function<void()> &)> _delegateQueue;
		Serializer _executor;
		
		const std::map<std::string,std::vector<Index>> _indexes;
		std::vector<weak_ptr<View>> _views;
		shared_ptr<IBackend> _backend;
		Comparator _comparator;

		friend class View;
	};
}

#endif