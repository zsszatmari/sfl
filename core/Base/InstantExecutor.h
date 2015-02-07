//
//  InstantExecutor.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/14/13.
//
//

#ifndef __G_Ear_Player__InstantExecutor__
#define __G_Ear_Player__InstantExecutor__

#include "IExecutor.h"

namespace Base
{
    class InstantExecutor : public IExecutor
    {
    public:
        virtual void addTask(Task &task);
        virtual void addTaskAndWait(Task &task);
    };
}

#endif /* defined(__G_Ear_Player__InstantExecutor__) */
