#include <functional>
#include "Db.h"
#include "ClientDb.h"
#include "IoService.h"
#include "MusicSort.h"
#include "SongSortOrder.h" 
#include "SongEntry.h"
#include "SongEntrySong.h"
#include "GearUtility.h"
#include "IApp.h"
#include "Bridge.h"
#include "RemoteDb.h"
#include "StringCompare.h"
#include "IoService.h"
#include "SongData.h"
#include "sfl/Prelude.h"

namespace Gear
{
#define method Db::

	using namespace ClientDb;
	using namespace sfl;

	static std::vector<ClientDb::Index> indexes()
	{
		std::vector<ClientDb::Index> ret;

		{
			vector<std::string> cols;
			// primary index
			// (source,entryId) is needed for mass deletion see Db::remove()
			// (maybe we need playlist::entryId as well? time shall tell)
			cols.push_back("source");
			cols.push_back("playlist");
			cols.push_back("entryId");
			cols.push_back("replica");

			ret.push_back(ClientDb::Index("Song","", cols));
		}
		{
			vector<std::string> cols;

			cols.push_back("source");
			cols.push_back("playlist");
			cols.push_back("entryId");
			cols.push_back("id");

			// replica: for queue?
			ret.push_back(ClientDb::Index("Song","other", cols));
		}
		/*
		cols = SongSortOrder::defaultSortOrder();
		ret.push_back(ClientDb::Index("Song","default", cols));

		cols.clear();
		// these are the columns that should be added as an extra, i.e. they are necessary, but not present in default sort order nor primary
		cols.push_back("albumArtUrl");
		cols.push_back("albumArtist");
		cols.push_back("albumId");
		cols.push_back("artistId");
		cols.push_back("creationDate");
		cols.push_back("durationMillis");
		cols.push_back("genre");
		cols.push_back("lastPlayed");
		cols.push_back("matchedKey");
		cols.push_back("playCount");
		cols.push_back("rating");
		ret.push_back(ClientDb::Index("Song","extra", cols));*/

		// generate indexes
		vector<std::string> bases = SongSortOrder::defaultSortOrder();
		bases.push_back("creationDate");
		bases.push_back("durationMillis");
		bases.push_back("genre");
		bases.push_back("lastPlayed");
		bases.push_back("playCount");
		bases.push_back("rating");
		for (auto &base : bases) {
			auto keysAndAscending = SongSortOrder::sortDescriptor(base, true).keysAndAscending();
			vector<string> keys;
			
			// we taught that this will speeed us up, but in face it does slow down. the cause
			// is that frequently (and in the prominent case of my library), 
			// with unions this is unusable in the end
			//keys.push_back("source");
			//keys.push_back("playlist");
			transform(keysAndAscending.begin(), keysAndAscending.end(), back_inserter(keys), [](std::pair<std::string, bool> &p){
				return p.first;
			});

			keys.push_back("source");
			keys.push_back("playlist");

			/*auto it = find_if(keys.begin(),keys.end(), [](const string &key){
				return key == "artist" || key == "album" || key == "title";
			});*/
			/*auto it = keys.begin() + 3;
			//keys.erase(it + 1, keys.end());*/
			#pragma message("TODO: we might be able to optimize on disk space, but the two solutions here are not enough (slow)")

			ret.push_back(ClientDb::Index("Song",base,keys));
		}

		{
			vector<std::string> cols;
			cols.push_back("albumArtUrl");
			cols.push_back("albumArtUrlHigh");
			cols.push_back("albumArtist");
			cols.push_back("albumId");
			cols.push_back("artistId");
			cols.push_back("matchedKey");
			//cols.push_back("comment");
			cols.push_back("offlinePath");   // store a '?' if intended to download but there is no path yet
			cols.push_back("offlineRatio");
			cols.push_back("dynamic");   // meaning: from a radio
			ret.push_back(ClientDb::Index("Song","extra", cols));
		}

		{
			vector<std::string> cols;
			cols.push_back("artist");
			cols.push_back("album");
			cols.push_back("title");
			cols.push_back("genre");
			//cols.push_back("comment");
			ret.push_back(ClientDb::Index("Song", "fulltext", cols, true));
		}

		SongData::ensureKeysPresent(concat(map([](const ClientDb::Index &index){return index.fields();},ret)));

		{
			vector<std::string> cols = {"source","playlistId"};
			ret.push_back(ClientDb::Index("Playlist", "", cols));
		}
		{
			vector<std::string> cols = {"category","parent","name","dynamic","editable"};
			ret.push_back(ClientDb::Index("Playlist", "other", cols));
		}
		return ret;
	}

	static function<void(const function<void()>)> queue()
	{
		return [](const function<void()> &f) {
			Io::get().post(f);
		};
	}

	method Db(const std::string &path) :
		_db(ClientDb::Db::openSync(path, indexes(), queue(), shared_ptr<IBackend>(new SqliteBackend()), &StringCompare::compareWithLength))
	{
		auto bridge = IApp::instance()->bridge();
		bridge->installSubscribe("view", [this](const Json::Value &arg, const function<void(const Json::Value &)> &callback){

			auto handle = arg.get("handle","").asString();
			auto entity = arg.get("entity","").asString();
			auto orderBy = RemoteDb::deserializeSortDescriptor(arg.get("orderBy", Json::nullValue));
			auto predicate = RemoteDb::deserializePredicate(arg.get("predicate", Json::nullValue));
			auto viewQueue = [](const function<void()> &f){
				Io::get().dispatch(f);
			};
			auto view = _db->view<SongData>(entity, predicate, orderBy, viewQueue, [callback](const shared_ptr<View<SongData>> &,ClientDb::IView::Event event,size_t offset,size_t size){
				//callback(event, offset, size);
				Json::Value value;
				value["event"] = static_cast<int>(event);
				value["offset"] = (uint32_t)offset;
				value["size"] = (uint32_t)size;
				callback(value);
			});

			_views[handle] = view;

		}, [this](const Json::Value &arg){
			auto handle = arg.get("handle","").asString();

			_views.erase(handle);
			
		}, Bridge::Privilege::RemoteControlParty);
#pragma message("TODO: remote control")
/*
		bridge->installGet("viewAt", [this](const Json::Value &arg){
			return viewAt(arg.get("handle","").asString(), arg.get("index",0).asInt());
		}, Bridge::Privilege::RemoteControlParty);
		bridge->installGet("viewSize",[this](const Json::Value &arg){
			return Json::Value((uint32_t)viewSize(arg.get("handle","").asString()));
		}, Bridge::Privilege::RemoteControlParty);
		bridge->installPost("viewSetVisibleRange",[this](const Json::Value &arg){
			viewSetVisibleRange(arg.get("handle","").asString(), arg.get("offset",0).asInt(), arg.get("size",0).asInt());
		}, Bridge::Privilege::RemoteControlParty);
		bridge->installGet("fetch",[this](const Json::Value &arg){
			auto result = fetchSync(arg.get("entity","").asString(), 
							RemoteDb::deserializePredicate(arg.get("predicate",Json::nullValue)),
                            RemoteDb::deserializeSortDescriptor(arg.get("sortDescriptor",Json::nullValue)),
							arg.get("limit",0).asInt(), arg.get("size",0).asInt(), arg.get("waitForChanges",false).asInt());
			Json::Value ret(Json::arrayValue);
			for (auto &e : result) {
				ret.append(e);
			}
			return ret;
		}, Bridge::Privilege::RemoteControlParty);
		bridge->installGet("fetchGroups",[this](const Json::Value &arg){
			auto result = fetchGroupings(arg.get("entity","").asString(),
							RemoteDb::deserializePredicate(arg.get("predicate",Json::nullValue)),
                            RemoteDb::deserializeSortDescriptor(arg.get("sortDescriptor",Json::nullValue)),
							RemoteDb::deserializeStrings(arg.get("fields",Json::nullValue)),
							RemoteDb::deserializeStrings(arg.get("groupBy",Json::nullValue)));
			Json::Value ret(Json::arrayValue);
			for (auto &e : result) {
				ret.append(e);
			}
			return ret;
		}, Bridge::Privilege::RemoteControlParty);
		bridge->installGet("fetchField",[this](const Json::Value &arg){
			auto result = fetchSync(arg.get("entity","").asString(), 
									RemoteDb::deserializePredicate(arg.get("predicate",Json::nullValue)),
									RemoteDb::deserializeSortDescriptor(arg.get("sortDescriptor",Json::nullValue)),
									arg.get("field","").asString());
			Json::Value ret(Json::arrayValue);
			for (auto &e : result) {
				ret.append(e);
			}
			return ret;
		}, Bridge::Privilege::RemoteControlParty);
		bridge->installGet("count",[this](const Json::Value &arg){
			auto result = count(arg.get("entity","").asString(), 
								RemoteDb::deserializePredicate(arg.get("predicate",Json::nullValue)));
			return Json::Value(result);
		}, Bridge::Privilege::RemoteControlParty);
*/
	}	

	void method upsert(const std::string &entity, const Json::Value &value)
	{
		_db->upsert(entity, value);
	}

	void method freeUpMemory()
	{
		_db->freeUpMemory();
	}	

	std::string method registerView(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const std::function<void(const std::function<void()> &)> &viewQueue, const std::function<void(ClientDb::IView::Event,size_t,size_t)> &callback)
	{
		auto view = _db->view<SongData>(entity, predicate, orderBy, viewQueue, [callback](const shared_ptr<View<SongData>> &,ClientDb::IView::Event event,size_t offset,size_t size){
			callback(event, offset, size);
		});

		auto handle = random_string(10);
		_views[handle] = view;
		return handle;
	}
	
	void method unregisterView(const std::string &handle)
	{
		_views.erase(handle);
	}

	SongData method viewAt(const std::string &handle, size_t index) const
	{
		auto it = _views.find(handle);
		if (it == _views.end()) {
			return Json::Value();
		}
		return it->second->at(index);
	}

	size_t method viewSize(const std::string &handle) const
	{
		auto it = _views.find(handle);
		if (it == _views.end()) {
			return 0;
		}
		return it->second->size();
	}

	void method viewSetVisibleRange(const std::string &handle, size_t offset, size_t size)
	{
		auto it = _views.find(handle);
		if (it == _views.end()) {
			return;
		}
		it->second->setVisibleRange(offset, size);
	}

	std::vector<Json::Value> method fetchSync(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, size_t limit, size_t offset, bool waitForChanges)
	{
		return _db->fetchSync(entity, predicate, orderBy, limit, offset, waitForChanges);
	}

	std::vector<std::string> method fetchSync(const std::string &entity, const Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const std::string &field)
	{
		return _db->fetchSync(entity, predicate, orderBy, field);
	}

	std::vector<Json::Value> method fetchGroupings(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy,
											const std::vector<std::string> &fetchFields, const std::vector<std::string> &groupBy)
	{
		return _db->fetchGroupings(entity, predicate, orderBy, fetchFields, groupBy);
	}

	uint32_t method count(const std::string &entity, const ClientDb::Predicate &predicate)
	{
		return _db->count(entity, predicate, Io::isCurrent() ? 0 : 1);
	}

	size_t method indexOf(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy, const Json::Value &item)
	{
		return _db->indexOf(entity, predicate, orderBy, item);
	}

	void method remove(const std::string &entity, const Json::Value &object)
	{
		_db->remove(entity, object);
	}

	void method remove(const std::string &entity, const Predicate &predicate)
	{
		_db->remove(entity, predicate);
	}

	int32_t method max(const std::string &entity, const ClientDb::Predicate &predicate, const std::string &field)
	{
		return _db->max(entity, predicate, field);
	}

	void method remove(const std::string &entity, const ClientDb::Predicate &predicate, 
					   const std::string &field, const std::vector<std::string> &values)
	{
		_db->remove(entity, predicate, field, values);
	}

	void method update(const std::string &entity, const ClientDb::Predicate &predicate, const Json::Value &updatedValues)
	{
		_db->update(entity, predicate, updatedValues);
	}

}
