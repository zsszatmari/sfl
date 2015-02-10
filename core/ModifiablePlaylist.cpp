//
//  ModifiablePlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#include "ModifiablePlaylist.h"
#include "MainExecutor.h"
#include "ISession.h"

namespace Gear {

#define method ModifiablePlaylist::

    shared_ptr<ModifiablePlaylist> method create(const string &playlistId, const string &name, const shared_ptr<SongManipulationSession> &session)
    {
        shared_ptr<ModifiablePlaylist> ret(new ModifiablePlaylist(playlistId, name, session));
        ret->init();
        return ret;
    }


    /*
    shared_ptr<ModifiablePlaylist> method create(const shared_ptr<ManagedObject> &underlyingObject, const shared_ptr<SongManipulationSession> &session)
    {
        auto playlist = shared_ptr<ModifiablePlaylist>(new ModifiablePlaylist(underlyingObject, session));
        playlist->init();
        return playlist;
    }
    
    shared_ptr<ModifiablePlaylist> method create(const string &playlistId, const shared_ptr<SongManipulationSession> &session)
    {
        auto playlist = shared_ptr<ModifiablePlaylist>(new ModifiablePlaylist(headlessObject(playlistId), session));
        playlist->init();
        return playlist;
    }*/
    
    method ModifiablePlaylist(const string &playlistId, const string &name, const shared_ptr<SongManipulationSession> &session) :
        StoredPlaylist(playlistId, name, session),
        _session(session),
        _removable(true)
    {
    }
    
    void method addSongEntries(const vector<SongEntry> &songEntries)
    {
        auto lSession = _session.lock();
        auto lplaylistId = playlistId();
        auto pThis = MEMORY_NS::static_pointer_cast<ModifiablePlaylist>(shared_from_this());

        vector<shared_ptr<ISong>> songs;
        transform(songEntries.begin(), songEntries.end(), back_inserter(songs),[](const SongEntry &entry){
            return entry.song();
        });
        
        if (lSession) {
            lSession->executor().addTask([lSession, songs, lplaylistId, pThis]{

                if (lplaylistId == "all") {
                    lSession->addAllAccessToLibrary(songs);
                } else {
                    lSession->addSongsToPlaylistAsync(lplaylistId, songs, [songs,pThis](const vector<SongEntry> &resultEntries){
                        if (songs.size() != resultEntries.size()) {
                            return;
                        }
                        
#pragma message("TODO: make adding songs to playlist work in the local db")
        /*                
                        MainExecutor::instance().addTask([pThis, resultEntries]{
                            pThis->addSongsLocally(resultEntries);
                        });*/
                    });
                }
            });
        } else {
            return;
        }
    }
    
    /*void method addSongsLocally(const vector<SongEntry> &songs)
    {
        storedSongArray()->addSongs(songs);
    }
    
    void method setSongsLocally(const vector<SongEntry> &songs)
    {
        storedSongArray()->addSongs(songs, false);
    }*/
    
    bool method removable() const
    {
        return _removable;
    }
    
    void method setRemovable(bool value)
    {
        _removable = value;
    }
    
    void method remove()
    {
        auto session = _session.lock();
        if (session) {
            session->removePlaylist(shared_from_this());
        }
    }
    
    void method removeSongs(const vector<SongEntry> &songs)
    {
        vector<string> songIds;
        vector<string> entryIds;
        songIds.reserve(songs.size());
        entryIds.reserve(songs.size());
        bool invalid = false;
        transform(songs.begin(), songs.end(), back_inserter(songIds), [&](const SongEntry &rhs)->std::string {
            
        	auto song = rhs.song();
        	if (!song) {
        		invalid = true;
        		return "";
        	}
            return song->uniqueId();
        });
        transform(songs.begin(), songs.end(), back_inserter(entryIds), [](const SongEntry &rhs){
            
            return rhs.entryId();
        });
        
        StoredPlaylist::removeSongs(songs);
        
        if (invalid) {
        	return;
        }

        auto session = _session.lock();
        if (session) {
            auto lPlaylistId = playlistId();
            // token seems to be not necessary
            //auto lToken = token();
            session->executor().addTask([session,entryIds, songIds, lPlaylistId]{
                session->deletePlaylistEntriesSync(entryIds, songIds, lPlaylistId);
            });
        }
    }
    
    bool method moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before)
    {
        if (!StoredPlaylist::moveSongs(entries, after, before)) {
            return false;
        }
        
        auto session = _session.lock();
        auto lPlaylistId = playlistId();
        session->executor().addTask([=]{
            session->changePlaylistOrderSync(lPlaylistId, entries, after, before);
        });
        
        return true;
    }
    
    void method setToken(const string &token)
    {
        _token = token;
    }
    
    const string & method token() const
    {
        return _token;
    }
    
    void method setName(const string &name)
    {
        StoredPlaylist::setName(name);
        
        auto session = _session.lock();
        if (session) {
            session->changePlaylistNameSync(name, playlistId());
        }
    }
}
