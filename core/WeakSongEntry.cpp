//
//  WeakSongEntry.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/6/13.
//
//

#include "WeakSongEntry.h"
#include "SongEntry.h"

namespace Gear
{
#define method WeakSongEntry::
    
    method WeakSongEntry()
    {
    }
    
    method WeakSongEntry(const SongEntry &entry) :
        _entryId(entry.entryId()),
        _song(entry.song())
    {
        
    }
    
    SongEntry method lock() const
    {
        auto song = _song.lock();
        if (song) {
            return SongEntry(song, _entryId);
        }
        return SongEntry();
    }
    
}
