//
//  PoolableExecutor.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__PoolableExecutor__
#define __G_Ear_core__PoolableExecutor__

#include "IExecutor.h"
#include "Thread.h"
#include <queue>
#include MUTEX_H
#include CONDITION_VARIABLE_H
#include MEMORY_H
#include UNIQUE_H
ALLOW_UNIQUE_PTR

namespace Base
{
    using std::queue;
    
    class PoolableExecutor final : public IExecutor
    {
    public:
        PoolableExecutor();
        virtual ~PoolableExecutor();
        
        virtual void addTask(Task &task);
        virtual void addTaskAndWait(Task &task);
        
    private:
		PoolableExecutor(const PoolableExecutor &rhs);
        const PoolableExecutor &operator=(const PoolableExecutor &rhs);
        
        void addTaskInt(const std::function<int(void)> &task);
        void addTaskIntAndWait(const std::function<int(void)> &task);
        
        MEMORY_NS::unique_ptr<Thread> currentThread;
        queue<std::function<void(void)>> tasks;
        THREAD_NS::mutex tasksMutex;
        THREAD_NS::condition_variable tasksWakeup;
        volatile bool done;
        volatile bool notified;
        
        void threadFunction();
    };
    
}


#endif /* defined(__G_Ear_core__PoolableExecutor__) */
