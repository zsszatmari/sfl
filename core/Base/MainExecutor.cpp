//
//  MainExecutor.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/14/13.
//
//

#include "MainExecutor.h"
#include "DefaultExecutor.h"
#include MEMORY_H

namespace Base
{
#define method MainExecutor::
    
    method MainExecutor()
    {
    }
    
    MainExecutor & method instance()
    {
        static MainExecutor executor;
        return executor;
    }
    
    void method addTask(Task &task)
    {
        DefaultExecutor::instance()->addTask(task);
    }
    
    void method addTaskAndWait(Task &task)
    {
        DefaultExecutor::instance()->addTaskAndWait(task);
    }
}