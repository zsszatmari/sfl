//
//  BackgroundExecutor.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#include "BackgroundExecutor.h"
#ifndef NON_APPLE
#include <dispatch/dispatch.h>
#else
#include "ExecutorPool.h"
#endif

namespace Base
{
#define method BackgroundExecutor::
    
    method BackgroundExecutor()
    {
    }
    
    BackgroundExecutor & method instance()
    {
        static BackgroundExecutor executor;
        return executor;
    }
    
    void method addTask(Task &task)
    {
#ifndef NON_APPLE
        Task copiedTask = task;
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            copiedTask();
        });
#else
        ExecutorPool::instance().addTask(task);
#endif
    }
    
    void method addTaskAndWait(Task &task)
    {
#ifndef NON_APPLE
        dispatch_sync(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            task();
        });
#else
        ExecutorPool::instance().addTaskAndWait(task);
#endif
    }
}