//
//  AddToPlaylistIntent.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#include "AddToPlaylistIntent.h"
#include "IPlaylist.h"

namespace Gear
{
#define method AddToPlaylistIntent::
    
    method AddToPlaylistIntent(const shared_ptr<IPlaylist> &playlist) :
        _playlist(playlist)
    {
    }
    
    const string method menuText() const
    {
        return _playlist->name();
    }
    
    void method apply(const vector<SongEntry> &songs) const
    {
        _playlist->addSongEntries(songs);
    }

    void method apply(const vector<shared_ptr<ISong>> &) const
    {
    }
}