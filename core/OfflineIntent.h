//
//  OfflineIntent.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 29/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__OfflineIntent__
#define __G_Ear_core__OfflineIntent__

#include "ISongIntent.h"
#include "ISong.h"
#include "CoreDebug.h"

namespace Gear
{
    class IPlaylist;
    
    class OfflineIntent final : public ISongIntent
    {
    public:
        // this may return nullptr
        static shared_ptr<OfflineIntent> create(const vector<SongEntry> &songs);
        
        virtual const string menuText() const;
        virtual void apply(const vector<SongEntry> &songs) const;
        
        static void apply(const vector<SongEntry> &songs, bool download);
        static std::string menuText(bool download);
        
    private:
        OfflineIntent(const vector<SongEntry> &songs);
        
        bool _download;
    };
}

#endif /* defined(__G_Ear_core__OfflineIntent__) */
