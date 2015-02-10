//
//  LibraryPlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/16/13.
//
//

#include "SongLibraryPlaylist.h"
#include "AllAccessToLibraryIntent.h"

namespace Gear
{
#define method SongLibraryPlaylist::
    
    shared_ptr<SongLibraryPlaylist> method create(const shared_ptr<ISession> &session)
    {
        auto ret = shared_ptr<SongLibraryPlaylist>(new SongLibraryPlaylist(session));
        ret->init();
        return ret;
    }
    
    method SongLibraryPlaylist(const shared_ptr<ISession> &session) :
    	IPlaylist(session),
        _name("My Library"),
        _playlistId("all"),
        _ordered(false)
    {
    }
    
    void method init()
    {
        auto session = _session.lock();
        if (session) {
            _songArray = shared_ptr<StoredSongArray>(new StoredSongArray(session, shared_from_this()));
        }
    }
    
    const shared_ptr<ISongArray> method songArray()
    {
        return _songArray;
    }
    
    const shared_ptr<StoredSongArray> method storedSongArray()
    {
        return _songArray;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {        
        bool okay = false;
        
        auto session = _session.lock();
        if (session) {
        	for (auto it = songs.begin() ; it != songs.end() ; ++it) {
        		auto &song = *it;
        		// if there is at least one song that is not already here...
                if (!_songArray->contains(song)) {
                    okay = true;
                    break;
                }
            }
        }
        if (okay) {
            return shared_ptr<ISongNoIndexIntent>(new AllAccessToLibraryIntent(session));
        } else {
            // even if we don't really add, user might miss the menu
            return shared_ptr<ISongNoIndexIntent>(new AllAccessToLibraryIntent(shared_ptr<ISession>()));
        }
    }
    
    const string method playlistId() const
    {
        return _playlistId;
    }
    
    void method setPlaylistId(const string &playlistId)
    {
        _playlistId = playlistId;
    }
    
    const string method name() const
    {
        return _name;
    }
    
    void method setName(const string &name)
    {
        _name = name;
    }
    
    void method removeSongs(const vector<SongEntry> &songs)
    {
        _songArray->removeSongs(songs);
    }
    
    bool method orderedPlaylist() const
    {
        return _ordered;
    }
    
    void method setOrdered(bool ordered)
    {
        _ordered = ordered;
    }
    
    void method addSongsLocally(const vector<SongEntry> &songs)
    {
        storedSongArray()->addSongs(songs);
    }
    
    /*
    void method removeSongsOtherThanLocally(const vector<SongEntry> &songs)
    {
        storedSongArray()->removeSongsOtherThan(songs);
    }*/

    shared_ptr<ISession> method session()
	{
    	return _session.lock();
	}
    
    bool method saveForOfflinePossible() const
    {
        return false;
    }
}
