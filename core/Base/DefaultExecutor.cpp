//
//  DefaultExecutor.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/14/13.
//
//

#include "DefaultExecutor.h"
#include "Environment.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

#ifndef NON_APPLE
#define DEFAULT_EXECUTOR_PRESENT

#include <dispatch/dispatch.h>

namespace Base {
    class DefaultExecutorImpl : public IExecutor
    {
    public:
        virtual void addTask(Task &task)
        {
            Task localTask = task;
            dispatch_async(dispatch_get_main_queue(), ^{
                localTask();
            });
        }
        
        virtual void addTaskAndWait(Task &task)
        {
            dispatch_sync(dispatch_get_main_queue(), ^{
                task();
            });
        }
    };
}

#endif


namespace Base
{
    
    

#define method DefaultExecutor::
    
    static shared_ptr<IExecutor> _instance;
    
    static shared_ptr<IExecutor> & defaultInstance()
    {
        static shared_ptr<IExecutor> def;
#ifdef DEFAULT_EXECUTOR_PRESENT
        if (def == nullptr) {
            def = shared_ptr<IExecutor>(new DefaultExecutorImpl());
        }
#endif
        return def;
    }
    
    shared_ptr<IExecutor> & method instance()
    {
        if (!_instance) {
            return defaultInstance();
        }
        
        return _instance;
    }
    
    void method registerInstance(const shared_ptr<IExecutor> &rhs)
    {
        _instance = rhs;
    }
}
