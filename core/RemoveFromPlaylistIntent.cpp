//
//  RemoveFromPlaylistIntent.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/17/13.
//
//

#include "RemoveFromPlaylistIntent.h"
#include "QueueSongArray.h"

namespace Gear
{
    using namespace Base;
    
#define method RemoveFromPlaylistIntent::
    
    method RemoveFromPlaylistIntent(const shared_ptr<ISession> &session) :
        _session(session)
    {
    }
    
    const string method menuText() const
    {
        return u("Remove from Playlist");
    }
    
    void method apply(const vector<SongEntry> &songs) const
    {
#pragma message("TODO: remove from playlist")
        QueueSongArray::instance()->willRemoveSongs(songs);
        //_playlist->removeSongs(songs);
    }
    
    bool method confirmationNeeded(const vector<SongEntry> &songs) const
    {
        if (songs.size() > 1) {
            return true;
        }
        return false;
    }
    
    string method confirmationText(const vector<SongEntry> &songs) const
    {
        return "Are you sure you want to remove selected songs from this playlist?";
    }
}
