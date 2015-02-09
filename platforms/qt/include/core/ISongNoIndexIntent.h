//
//  ISongNoIndexIntent.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/17/13.
//
//

#ifndef __G_Ear_Player__ISongNoIndexIntent__
#define __G_Ear_Player__ISongNoIndexIntent__

#include "ISongIntent.h"
#include "stdplus.h"
#include MEMORY_H

namespace Gear
{
    class ISong;
    
    class ISongNoIndexIntent : public ISongIntent
    {
    public:
        virtual void apply(const vector<SongEntry> &songs) const;
        virtual void apply(const vector<MEMORY_NS::shared_ptr<ISong>> &songs) const = 0;
    };
}

#endif /* defined(__G_Ear_Player__ISongNoIndexIntent__) */
