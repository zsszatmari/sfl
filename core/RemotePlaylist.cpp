#include "RemotePlaylist.h"
#include "SortedSongArray.h"
#include "RemoteDb.h"

namespace Gear
{
#define method RemotePlaylist::

	vector<shared_ptr<IPlaylist>> method deserialize(const Json::Value &array)
	{
		vector<shared_ptr<IPlaylist>> ret;

		if (array.isArray()) {
			for (int i = 0 ; i < array.size() ; ++i) {
				const Json::Value &item = array[i];
				shared_ptr<RemotePlaylist> playlist(new RemotePlaylist());
				playlist->_playlistId = item.get("playlistId","").asString();
				playlist->_name = item.get("name","").asString();
				playlist->_songArray = 
					shared_ptr<ISongArray>(new SortedSongArray(RemoteDb::instance(), RemoteDb::deserializePredicate(item.get("inherentPredicate",Json::Value()))));
				ret.push_back(playlist);
			}
		}

		return ret;
	}

	Json::Value method serialize(const vector<shared_ptr<IPlaylist>> &playlists)
	{
		Json::Value ret(Json::arrayValue);
		for (auto &playlist : playlists) {
			if (playlist) {
				Json::Value item(Json::objectValue);
				item["playlistId"] = playlist->playlistId();
				item["name"] = playlist->name();
				item["inherentPredicate"] = RemoteDb::serialize(MEMORY_NS::dynamic_pointer_cast<SortedSongArray>(playlist->songArray())->inherentPredicate());
				ret.append(item);
			}
		}
		return ret;	
	}

	method RemotePlaylist() :
	// session? does it make sense?
		IPlaylist(weak_ptr<ISession>())
	{
	}

	const string method playlistId() const
	{
		return _playlistId;
	}

    const string method name() const
    {
    	return _name;
    }

    void method setName(const string &name)
    {
    }

    const shared_ptr<ISongArray> method songArray()
    {
    	return _songArray;
    }

    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
    	return nullptr;
    }

    bool method saveForOfflinePossible() const
    {
    	return false;
    }
}