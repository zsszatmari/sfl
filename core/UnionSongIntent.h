//
//  UnionSongIntent.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/5/13.
//
//

#ifndef __G_Ear_Player__UnionSongIntent__
#define __G_Ear_Player__UnionSongIntent__

#include "ISongIntent.h"
#include "stdplus.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    class UnionSongIntent final : public ISongIntent
    {
    public:
        UnionSongIntent(const vector<shared_ptr<ISongIntent>> &intents);
        
        virtual const string menuText() const;
        virtual void apply(const vector<SongEntry> &songs) const;
        virtual bool confirmationNeeded(const vector<SongEntry> &songs) const;
        virtual string confirmationText(const vector<SongEntry> &songs) const;
        
    private:
        const vector<shared_ptr<ISongIntent>> _intents;
    };
}

#endif /* defined(__G_Ear_Player__UnionSongIntent__) */
