//
//  InstantExecutor.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/14/13.
//
//

#include "InstantExecutor.h"

namespace Base
{
#define method InstantExecutor::
    
    void method addTask(Task &task)
    {
        task();
    }
    
    void method addTaskAndWait(Task &task)
    {
        task();
    }
}