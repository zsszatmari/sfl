//
//  SongGoogleLibraryPlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/17/13.
//
//

#include "SongGoogleLibraryPlaylist.h"
#include "GooglePlaySession.h"

namespace Gear
{
#define method SongGoogleLibraryPlaylist::
    
    shared_ptr<SongGoogleLibraryPlaylist> method create(const shared_ptr<GooglePlaySession> &session)
    {
        shared_ptr<SongGoogleLibraryPlaylist> ret(new SongGoogleLibraryPlaylist(session));
        ret->init();
        return ret;
    }
    
    method SongGoogleLibraryPlaylist(const shared_ptr<GooglePlaySession> &session) :
        SongLibraryPlaylist(session)
    {
    }
    
    void method removeSongs(const vector<SongEntry> &songs)
    {
        SongLibraryPlaylist::removeSongs(songs);
    
        auto sSession = _session.lock();
        auto session = MEMORY_NS::static_pointer_cast<GooglePlaySession>(sSession);
        if (session) {
            vector<string> songIds;
            songIds.reserve(songs.size());
            transform(songs.begin(), songs.end(), back_inserter(songIds), [](const SongEntry &rhs){
                return rhs.song()->uniqueId();
            });
            
            session->executor().addTask([session, songIds]{
                
                vector<string> entryIds;
                session->deletePlaylistEntriesSync(entryIds, songIds, "all");
            });
        }
    }
    
    bool method saveForOfflinePossible() const
    {
        return false;
    }
}
