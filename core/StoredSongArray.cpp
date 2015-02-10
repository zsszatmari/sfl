//
//  StoredSongArray.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/10/13.
//
//

#include "StoredSongArray.h"
#include "GooglePlaySession.h"
#include "PlaylistSession.h"
#include "RemoveFromPlaylistIntent.h"
#include "RemoveFromLibraryIntent.h"
#include "ISongNoIndexIntent.h"
#include "OfflineIntent.h"
#include "AllAccessToLibraryIntent.h"

namespace Gear
{
#define method StoredSongArray::
    
    method StoredSongArray(const shared_ptr<ISession> &session, const shared_ptr<IPlaylist> playlist) :
        SessionSongArray(session, ClientDb::Predicate::compositeAnd(ClientDb::Predicate("source",session->sessionIdentifier()), ClientDb::Predicate("playlist", playlist->playlistId()))),
        _playlist(playlist)
    {
    }
    
    void method addSongs(const vector<SongEntry> &songs, bool append)
    {
        setAllSongs(songs, append, true);
    }
    
    const bool method orderedArray() const
    {
        auto playlist = _playlist.lock();
        if (playlist) {
            return playlist->orderedPlaylist();
        } else {
            return false;
        }
    }
}
