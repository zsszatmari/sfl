//
//  RemoveFromLibraryIntent.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/17/13.
//
//

#include "RemoveFromLibraryIntent.h"
#include "SongManipulationSession.h"
#include "IPlaylist.h"
#include "QueueSongArray.h"

namespace Gear
{
    using namespace Base;
    
#define method RemoveFromLibraryIntent::
    
    method RemoveFromLibraryIntent(const shared_ptr<ISession> &session) :
        _session(session)
    {
    }
    
    const string method menuText() const
    {
        return u("Remove from Library");
    }
    
    void method apply(const vector<SongEntry> &songs) const
    {
    	auto s = _session.lock();
    	if (s) {
            QueueSongArray::instance()->willRemoveSongs(songs);
#pragma("TODO: remove song from library")
            //s->libraryPlaylist()->removeSongs(songs);
    	}
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
        return "Are you sure you want to remove selected songs from the library?";
    }
}
