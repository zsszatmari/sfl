//
//  PlaylistDeleteIntent.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 06/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <sstream>
#include "PlaylistDeleteIntent.h"
#include "IPlaylist.h"

namespace Gear
{
#define method PlaylistDeleteIntent::
    
    method PlaylistDeleteIntent(const shared_ptr<IPlaylist> &playlist) :
        _playlist(playlist)
    {
    }
    
    const std::string method menuText() const
    {
        return "Delete";
    }
    
    const std::string method confirmationText() const
    {
        auto playlist = _playlist.lock();
        if (!playlist) {
            return "";
        }
        
        std::stringstream str;
        str << "Are you sure you wish to delete " << playlist->name() << "?";
        return str.str();
    }
    
    void method apply()
    {
        auto playlist = _playlist.lock();
        if (playlist) {
            playlist->remove();
        }
    }
}