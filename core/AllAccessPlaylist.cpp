//
//  AllAccessPlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#include "AllAccessPlaylist.h"
#include "AllAccessSongArray.h"

namespace Gear
{
#define method AllAccessPlaylist::
    
    method AllAccessPlaylist(const shared_ptr<ISession> &session) :
        BasicPlaylist(session, "All Access", "free", AllAccessSongArray::create(session))
    {
    }
    
    bool method saveForOfflinePossible() const
    {
        return false;
    }
}
