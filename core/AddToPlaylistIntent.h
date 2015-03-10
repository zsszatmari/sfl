//
//  AddToPlaylistIntent.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#ifndef __G_Ear_Player__AddToPlaylistIntent__
#define __G_Ear_Player__AddToPlaylistIntent__

#include "stdplus.h"
#include MEMORY_H
#include "ISongNoIndexIntent.h"
 
namespace Gear
{
    class IPlaylist;

    class AddToPlaylistIntent final : public ISongNoIndexIntent
    {
    public:
        AddToPlaylistIntent(const MEMORY_NS::shared_ptr<IPlaylist> &playlist);
        virtual const string menuText() const;
        virtual void apply(const vector<SongEntry> &songs) const;
        virtual void apply(const vector<MEMORY_NS::shared_ptr<ISong>> &) const;
    
    private:
        MEMORY_NS::shared_ptr<IPlaylist> _playlist;
    };
}

#endif /* defined(__G_Ear_Player__AddToPlaylistIntent__) */
