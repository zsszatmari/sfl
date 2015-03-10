//
//  PlaylistDeleteIntent.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 06/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__PlaylistDeleteIntent__
#define __G_Ear_core__PlaylistDeleteIntent__

#include "IPlaylistIntent.h"
#include "stdplus.h"
#include WEAK_H
using MEMORY_NS::shared_ptr;
using MEMORY_NS::weak_ptr;

namespace Gear
{
    class IPlaylist;
    
    class PlaylistDeleteIntent : public IPlaylistIntent
    {
    public:
        PlaylistDeleteIntent(const shared_ptr<IPlaylist> &playlist);
        virtual const std::string menuText() const;
        virtual const std::string confirmationText() const;
        virtual void apply();
        
    private:
        weak_ptr<IPlaylist> _playlist;
    };
}

#endif /* defined(__G_Ear_core__PlaylistDeleteIntent__) */
