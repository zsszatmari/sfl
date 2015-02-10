//
//  spinlock.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 21/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "spinlock.h"

namespace Base
{
    /*
#define method spinlock::
    
    method spinlock() : state_(Unlocked)
    {
    }
    
    void method lock()
    {
        while (state_.exchange(Locked, memory_order::memory_order_acquire) == Locked) {
            // busy-wait
        }
    }
    
    void method unlock()
    {
        state_.store(Unlocked, memory_order::memory_order_release);
    }*/
}
