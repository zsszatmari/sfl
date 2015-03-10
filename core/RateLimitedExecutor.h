//
//  RateLimitedExecutor.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/23/13.
//
//

#ifndef __G_Ear_Player__RateLimitedExecutor__
#define __G_Ear_Player__RateLimitedExecutor__

#include "SerialExecutor.h"

namespace Gear
{
    class RateLimitedExecutor : public Base::SerialExecutor
    {
    public:
        RateLimitedExecutor(float limit);
        virtual void addTask(Base::IExecutor::Task &task);
        virtual void addTaskAndWait(Base::IExecutor::Task &task);
        
    private:
        RateLimitedExecutor(); // delete
        
        const int _limitMillisec;
    };;
}

#endif /* defined(__G_Ear_Player__RateLimitedExecutor__) */
