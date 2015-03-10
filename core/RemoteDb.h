#include "IDb.h"

namespace Gear
{
	class RemoteDb final : public IDb
	{
	public:
		static shared_ptr<IDb> instance();

		virtual std::string registerView(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const std::function<void(const std::function<void()> &)> &viewQueue, const std::function<void(ClientDb::IView::Event,size_t,size_t)> &callback);
		virtual void unregisterView(const std::string &handle);

		virtual SongData viewAt(const std::string &handle, size_t index) const;
		virtual size_t viewSize(const std::string &handle) const;
		virtual void viewSetVisibleRange(const std::string &handle, size_t offset, size_t size);

		virtual std::vector<Json::Value> fetchSync(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, size_t limit = 0, size_t offset = 0, bool waitForChanges = true);
		virtual std::vector<std::string> fetchSync(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const std::string &field);
		virtual uint32_t count(const std::string &entity, const ClientDb::Predicate &predicate);

		virtual std::vector<Json::Value> fetchGroupings(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy,
												const std::vector<std::string> &fetchFields, const std::vector<std::string> &groupBy);

		/* possibly not needed for remote:*/
		virtual size_t indexOf(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const Json::Value &item);
		virtual void remove(const std::string &entity, const ClientDb::Predicate &predicate);
		virtual void remove(const std::string &entity, const Json::Value &object);
		virtual int32_t max(const std::string &entity, const ClientDb::Predicate &predicate, const std::string &field);	


		static Json::Value serialize(const ClientDb::Predicate &predicate);
		static ClientDb::Predicate deserializePredicate(const Json::Value &json);

		static Json::Value serialize(const ClientDb::SortDescriptor &SortDescriptor);
		static ClientDb::SortDescriptor deserializeSortDescriptor(const Json::Value &json);

		static Json::Value serialize(const std::vector<std::string> &strings);
		static std::vector<std::string> deserializeStrings(const Json::Value &json);

	private:
		RemoteDb();

		std::map<std::string, Json::Value> _views;
	};
}