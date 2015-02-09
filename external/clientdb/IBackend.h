#ifndef IBACKEND_H
#define IBACKEND_H

#include <string>
#include <map>
#include <cstdint>
#include "Index.h"
#include "Predicate.h"
#include "Comparator.h"

namespace ClientDb
{
	class IBackend
	{
	public:
		virtual bool open(const std::string &path, const std::map<std::string,std::vector<Index>> &indexes, Comparator comparator = nullptr) = 0;
		virtual void destroy(const std::string &path) = 0;
		// inserts or updates value. returns nullValue if not already present, otherwise old value
		virtual Json::Value upsert(const std::string &entity, Json::Value &value, const std::function<void(const Json::Value &, Json::Value &)> &oldObjectHandler) = 0;
		virtual void remove(const std::string &entity, const Predicate &predicate, int connectionId = 0) = 0;
		virtual std::vector<Json::Value> remove(const std::string &entity, const Predicate &predicate, const std::string &field, const std::vector<std::string> &values, int connectionId) = 0;
		virtual size_t count(const std::string &entity, const Predicate &predicate, int connectionId) = 0;
		virtual int32_t max(const std::string &entity, const Predicate &predicate, const std::string &field) = 0;
		virtual std::vector<Json::Value> fetch(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const Json::Value &justBefore, size_t limit, size_t offset = 0, int connectionId = 0) = 0;
		virtual std::vector<std::string> fetch(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const std::string &field) = 0;
		virtual std::vector<Json::Value> fetchGroupings(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy,
											const std::vector<std::string> &fetchFields, const std::vector<std::string> &groupBy) = 0;
	
		virtual void freeUpMemory(int connectionId) = 0;

		virtual ~IBackend() {}

	private:
	};
};

#endif