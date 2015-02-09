//
//  CreatePlaylistIntent.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#include "CreatePlaylistIntent.h"

namespace Gear
{
#define method CreatePlaylistIntent::
    
    method CreatePlaylistIntent(const shared_ptr<PlaylistSession> &session) :
        _session(session)
    {
    }

    const string method menuText() const
    {
        return u("New Playlist");
    }
    
    void method apply(const vector<shared_ptr<ISong>> &songs) const
    {
        _session->createUserPlaylist(songs);
    }
}