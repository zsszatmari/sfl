//
//  QueueIntent.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 29/04/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__QueueIntent__
#define __G_Ear_core__QueueIntent__

#include "ISongIntent.h"

namespace Gear
{
    class QueueIntent final : public ISongIntent
    {
    public:
        enum class Flavour
        {
            EnqueueNext,
            EnqueueLast,
            Remove
        };
        
        QueueIntent(const Flavour flavour);
        
        virtual const string menuText() const;
        virtual void apply(const vector<SongEntry> &songs) const;
    
    private:
        const Flavour _flavour;
    };
}

#endif /* defined(__G_Ear_core__QueueIntent__) */
