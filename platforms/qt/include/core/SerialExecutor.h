//
//  SerialExecutor.h
//  Base
//
//  Created by Zsolt Szatmári on 5/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __Base__SerialExecutor__
#define __Base__SerialExecutor__

#include "IExecutor.h"
#include "PoolableExecutor.h"
#include <queue>
#include MUTEX_H
#include THREAD_H
#include MEMORY_H
using MEMORY_NS::shared_ptr; 

namespace Base
{
    using std::queue;
    
    class core_export SerialExecutor : public IExecutor
    {                
    public:
        SerialExecutor();
        virtual ~SerialExecutor();
        
        virtual void addTask(Task &task);
        virtual void addTaskAndWait(Task &task);
        
    private:
		SerialExecutor(const SerialExecutor &rhs);
        const SerialExecutor &operator=(const SerialExecutor &rhs);
        
        class State final {
        public:
            State();
            
            THREAD_NS::mutex _tasksMutex;
            bool _running;
            THREAD_NS::thread::id _threadId;
            std::queue<MutableTask> _tasks;
        };
        
        shared_ptr<State> _state;
    };
        
}

#endif /* defined(__Base__SerialExecutor__) */
