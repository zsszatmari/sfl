//
//  spinlock.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 21/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__spinlock__
#define __G_Ear_core__spinlock__

#include "stdplus.h"

namespace Base
{
    // spinlock is dangerous, especiallyon a uni-processor system, don't use it.
    /*class spinlock
    {
    public:
        spinlock();
        
        void lock();
        void unlock();
        
    private:
        typedef enum {Locked, Unlocked} LockState;
        atomic<LockState> state_;
    };*/
}



#endif /* defined(__G_Ear_core__spinlock__) */
