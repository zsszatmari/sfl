//
//  DragManager.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/31/13.
//
//

#include "DragManager.h"

namespace Gear
{
    
#define method DragManager::
    
    method DragManager()
    {
    }
    
    void method dragSongs(const weak_ptr<ISongArray> source, const vector<SongEntry> &songs)
    {
        _draggedEntries = songs;
        _songsSource = source;
    }
    
    const vector<SongEntry> & method draggedEntries() const
    {
        return _draggedEntries;
    }
    
    const weak_ptr<ISongArray> & method songsSource() const
    {
        return _songsSource;
    }
}