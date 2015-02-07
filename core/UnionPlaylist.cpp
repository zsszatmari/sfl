//
//  UnionPlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/27/13.
//
//

#include "stdplus.h"
#include "UnionPlaylist.h"
#include "UnionSongArray.h"

namespace Gear
{
#define method UnionPlaylist::
    
    method UnionPlaylist(const string &playlistId, const string &name) :
    	IPlaylist(weak_ptr<ISession>()),
        _playlistId(playlistId),
        _name(name)
    {
    }
    
    void method setPlaylists(vector<shared_ptr<IPlaylist>> playlists)
    {
    	if (_playlists != playlists) {
			_playlists = playlists;
			recompute();
    	}
    }
    
    const string method playlistId() const
    {
        return _playlistId;
    }
    
    const string method name() const
    {
        if (_playlists.size() == 1) {
            return _playlists.at(0)->name();
        }
        return _name;
    }
    
    void method setName(const string &name)
    {
        _name = name;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
    	for (auto it = _playlists.begin() ; it != _playlists.end() ; ++it) {
    		auto &playlist = *it;
            auto ret = playlist->dragIntentTo(songs);
            if (ret) {
                return ret;
            }
        }
        return shared_ptr<ISongNoIndexIntent>();
    }
    
    const shared_ptr<ISongArray> method songArray()
    {
        return unionArray();
    }
    
    const shared_ptr<SortedSongArray> method unionArray()
    {
        auto array = _songArray.lock();
        if (array) {
            return array;
        }
        
        shared_ptr<UnionSongArray> ret = UnionSongArray::create();
        _songArray = ret;
        recompute();
        
        return ret;
    }
    
    void method recompute()
    {
        auto array = _songArray.lock();
        if (array) {
            vector<shared_ptr<SortedSongArray>> arrays;
            arrays.reserve(_playlists.size());
            for (auto &playlist : _playlists) {
                auto sortedArray = MEMORY_NS::dynamic_pointer_cast<SortedSongArray>(playlist->songArray());
                if (sortedArray) {
                    arrays.push_back(sortedArray);
                }
            }
            
            array->setSongArrays(arrays);
        }
    }

    bool method saveForOfflinePossible() const
    {
        for (auto &p : _playlists) {
            if (p->saveForOfflinePossible()) {
                return true;
            }
        }
        return false;
    }

    bool method reorderable() const
    {
        return false;
    }
}
