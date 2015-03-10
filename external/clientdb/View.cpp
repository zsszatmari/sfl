#include <View.h>
#include "ClientDb.h"

namespace ClientDb
{
	uint32_t IView::fetchCount(const shared_ptr<Db> &db, const std::string &entity, const Predicate &predicate, int connectionId)
	{
		// this must be in separate compuliation unit otherwise mingw can't compile because of interdependency
		return db->count(entity, predicate, connectionId);
	}

	std::vector<Json::Value> IView::fetch(const shared_ptr<Db> &db, const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const Json::Value &justBefore, size_t limit, size_t offset, int connectionId)
	{
		return db->_backend->fetch(entity, predicate, orderBy, justBefore, limit, offset, 1);
	}

}