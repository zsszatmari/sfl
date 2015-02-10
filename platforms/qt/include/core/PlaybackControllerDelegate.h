//
//  PlaybackControllerDelegate.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/21/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__PlaybackControllerDelegate__
#define __G_Ear_core__PlaybackControllerDelegate__

#include "stdplus.h"

namespace Gear
{
    class Isong;
    
    class PlaybackControllerDelegate
    {
    public:
        virtual ~PlaybackControllerDelegate() {}
        
        virtual void setElapsedTime(float elapsed) = 0;
        virtual void setRemainingTime(float remaining) = 0;
        virtual void changedRatio(float ratio) = 0;
        virtual void songFinished(shared_ptr<Gear::ISong> songPlayed) = 0;
        
    };
}

#endif /* defined(__G_Ear_core__PlaybackControllerDelegate__) */
