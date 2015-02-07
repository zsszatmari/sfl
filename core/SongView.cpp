#include "SongView.h"
#include "IDb.h"

namespace Gear
{
#define method SongView::

	using std::function;
	using MEMORY_NS::shared_ptr;

	method SongView()
	{
	}

	method SongView(const shared_ptr<IDb> &db, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const function<void(const function<void()> &)> &viewQueue, const function<void(const SongView &songView, SongView::Event,size_t,size_t)> &callback) :
		_db(db),
		_viewHandle(new std::string)
	{
		auto viewHandle = _viewHandle;
		*viewHandle = db->registerView("Song", predicate, orderBy, viewQueue, [callback,viewHandle,db](ClientDb::View::Event event, size_t offset, size_t count){
			SongView copy;
			copy._db = db;
			copy._viewHandle = viewHandle;
			callback(copy, static_cast<SongView::Event>(event),offset,count);
		});
	}

	method ~SongView()
	{
		if (_viewHandle.unique()) {
			_db->unregisterView(*_viewHandle);
		}
	}

	bool method operator==(const SongView &rhs) const
	{
		return _viewHandle == rhs._viewHandle;
	}
	
	static SongEntry dummyEntry()
	{
		Json::Value values;
		values["artist"] = "Bimm Bamm";
		values["album"] = "Bommm Bumm";
		values["title"] = "Timm Tamm";
		
		SongEntry entry(values);

		return entry;
	}

	SongEntry method at(size_t index) const
	{
//		static auto debugEntry = dummyEntry();
//		return debugEntry;

		return SongEntry(_db->viewAt(*_viewHandle,index));
	}
		
	size_t method size() const
	{
		if (!_db) {
			return 0;
		}
		return _db->viewSize(*_viewHandle);
	}

	void method setVisibleRange(size_t offset, size_t size)
	{
		if (!_db) {
			return;
		}
		_db->viewSetVisibleRange(*_viewHandle, offset, size);
	}
}
