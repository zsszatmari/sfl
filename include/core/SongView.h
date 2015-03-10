#ifndef SONGVIEW_H
#define SONGVIEW_H

#include <functional>
#include "SongEntry.h"

namespace ClientDb
{
	class Predicate;
	class SortDescriptor;
}

namespace Gear
{
	class IDb;

	class SongView final
	{
	public:
		enum class Event
		{
			Inserted,
			Deleted,
			Updated,
			Reload
		};

		SongView();
		SongView(const shared_ptr<IDb> &db, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const std::function<void(const std::function<void()> &)> &viewQueue, const std::function<void(const SongView &,Event,size_t,size_t)> &callback);
		~SongView();

		SongEntry at(size_t index) const;
		size_t size() const;
		void setVisibleRange(size_t offset, size_t size);
		bool operator==(const SongView &) const;

	private:
		MEMORY_NS::shared_ptr<IDb> _db;
		MEMORY_NS::shared_ptr<std::string> _viewHandle;
	};
}

#endif