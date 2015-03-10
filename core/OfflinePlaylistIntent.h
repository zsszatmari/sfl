//
//  OfflinePlaylistIntent.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 01/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__OfflinePlaylistIntent__
#define __G_Ear_core__OfflinePlaylistIntent__

#include "stdplus.h"
#include "IPlaylistIntent.h"
#include WEAK_H
using MEMORY_NS::weak_ptr;
using MEMORY_NS::shared_ptr;

namespace Gear
{
    class ISongArray;
    
    class OfflinePlaylistIntent : public IPlaylistIntent
    {
    public:
        OfflinePlaylistIntent(const shared_ptr<const ISongArray> &, bool download, const std::string &name);
        
        virtual const std::string menuText() const;
        virtual const std::string confirmationText() const;
        virtual void apply();
        
    private:
        weak_ptr<const ISongArray> _songArray;
        bool _download;
        std::string _name;
    };
}

#endif /* defined(__G_Ear_core__OfflinePlaylistIntent__) */
