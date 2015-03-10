#ifndef IDB_H
#define IDB_H

#include "View.h"
#include "SongData.h"

namespace ClientDb
{
	class Predicate;
	class SortDescriptor;
}

namespace Gear
{
	class IDb
	{
	public:
		virtual ~IDb();
		// returns a handle

		virtual std::string registerView(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const std::function<void(const std::function<void()> &)> &viewQueue, const std::function<void(ClientDb::IView::Event,size_t,size_t)> &callback) = 0;
		virtual void unregisterView(const std::string &handle) = 0;

		virtual SongData viewAt(const std::string &handle, size_t index) const = 0;
		virtual size_t viewSize(const std::string &handle) const = 0;
		virtual void viewSetVisibleRange(const std::string &handle, size_t offset, size_t size) = 0;

		virtual std::vector<Json::Value> fetchSync(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, size_t limit = 0, size_t offset = 0, bool waitForChanges = true) = 0;
		virtual std::vector<std::string> fetchSync(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const std::string &field) = 0;
		virtual uint32_t count(const std::string &entity, const ClientDb::Predicate &predicate) = 0;

		virtual std::vector<Json::Value> fetchGroupings(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy,
												const std::vector<std::string> &fetchFields, const std::vector<std::string> &groupBy) = 0;

		/* possibly not needed for remote:*/

		// needed for queue calculation
		virtual size_t indexOf(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const Json::Value &item) = 0;
		virtual void remove(const std::string &entity, const ClientDb::Predicate &predicate) = 0;
		virtual void remove(const std::string &entity, const Json::Value &object) = 0;
		// needed for refreshing db from service
		virtual int32_t max(const std::string &entity, const ClientDb::Predicate &predicate, const std::string &field) = 0;		
	};
}

#endif
