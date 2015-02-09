#ifndef SQLITEBACKEND_H
#define SQLITEBACKEND_H

#include <functional>
#include "stdplus.h"
#include "IBackend.h"
#include "sqlite3.h"

namespace ClientDb
{
	class SqliteStatement;

// this flag makes everything simpler to reason about, it is still unclear whether without it there would
// be a performance advantage  
#define SERIALIZED_DATABASE

	class SqliteBackend final : public IBackend
	{
	public:
		SqliteBackend();
		virtual bool open(const std::string &path, const std::map<std::string,std::vector<Index>> &indexes, Comparator comparator);
		virtual void destroy(const std::string &path);
		// inserts or updates value. returns nullValue if not already present, otherwise old value
		virtual Json::Value upsert(const std::string &entity, Json::Value &value, const std::function<void(const Json::Value &, Json::Value &)> &oldObjectHandler);
		virtual void remove(const std::string &entity, const Predicate &predicate, int connectionId = 0);
		virtual std::vector<Json::Value> remove(const std::string &entity, const Predicate &predicate, const std::string &field, const std::vector<std::string> &values, int connectionId);
		virtual size_t count(const std::string &entity, const Predicate &predicate, int connectionId);
		virtual std::vector<Json::Value> fetch(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const Json::Value &justBefore, size_t limit, size_t offset, int connectionId = 0);
		virtual std::vector<std::string> fetch(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const std::string &field);
		int32_t max(const std::string &entity, const Predicate &predicate, const std::string &field);
		virtual void freeUpMemory(int connectionId) override final;
		virtual std::vector<Json::Value> fetchGroupings(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy,
											const std::vector<std::string> &fetchFields, const std::vector<std::string> &groupBy);
	
		virtual ~SqliteBackend();

		std::pair<std::string,std::function<void(SqliteStatement &)>> whereClause(const std::string &entity, const Predicate &predicate, const std::string &prefix = "WHERE");

	private:
		SqliteBackend &operator=(const SqliteBackend &); // delete
		SqliteBackend(const SqliteBackend &); // delete

		bool execute(const std::string &statement, bool throwOnError = true, int connectionId = 0);

		// We use two separate connections, number 0 is ought to be used in the 
		// background, while 1 is for foreground use. this is because we want to avoid background operations
		// blocking the main (ui) thread 
		static const int connectionCount = 2;
		sqlite3 *_db[connectionCount];
		std::map<std::string,std::vector<Index>> _indexes;
		std::map<std::string,std::vector<std::string>> _fullTextIndex;
		std::map<std::string,std::vector<std::string>> _columns;
	};
}

#endif