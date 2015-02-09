//
//  QueuePlaylist.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 28/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "QueuePlaylist.h"
#include "QueueSongArray.h"
#include "AddToPlaylistIntent.h"

namespace Gear
{
#define method QueuePlaylist::
    
    shared_ptr<QueuePlaylist> method create()
    {
        return shared_ptr<QueuePlaylist>(new QueuePlaylist());
    }

    method QueuePlaylist() :
        BasicPlaylist(weak_ptr<ISession>(), "Queue", "queue", QueueSongArray::instance())
    {
    }
    
    bool method editable() const
    {
        return true;
    }
    
    bool method moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before)
    {
        return QueueSongArray::instance()->moveSongs(entries, after, before);
    }

    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>(new AddToPlaylistIntent(shared_from_this()));
    }

    void method addSongEntries(const vector<SongEntry> &songs)
    {
        QueueSongArray::instance()->enqueue(songs, false);
    }

    bool method nameEditable() const
    {
        return false;
    }
}
