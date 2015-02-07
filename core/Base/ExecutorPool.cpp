//
//  ExecutorPool.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "ExecutorPool.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Base
{
#define method ExecutorPool::

    using THREAD_NS::lock_guard;
    using THREAD_NS::mutex;
    
    ExecutorPool & method instance()
    {
        static ExecutorPool *pool = new ExecutorPool();
        return *pool;
    }
    
    method ExecutorPool()
    {
    }

    void method addTask(Task &task)
    {
        shared_ptr<PoolableExecutor> executor;
        {
            lock_guard<mutex> l(_idleMutex);
            if (!_idle.empty()) {
                executor = _idle.back();
                _idle.pop_back();
            }
        }
        
        if (!executor) {
            executor = shared_ptr<PoolableExecutor>(new PoolableExecutor());
        }
        
        executor->addTask([executor, task, this]{
            
            task();
            
            lock_guard<mutex> l(_idleMutex);
            _idle.push_back(executor);
        });
    }
    
    void method addTaskAndWait(Task &task)
    {
        // it would make little sense to wait for a task on an unknown thread...
        task();
    }
}
