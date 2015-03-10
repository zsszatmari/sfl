//
//  SerialExecutor.cpp
//  Base
//
//  Created by Zsolt Szatmári on 5/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <memory>
#include "Environment.h"
#include "SerialExecutor.h"
#include "ExecutorPool.h"
#include "stdplus.h"


namespace Base
{
#define method SerialExecutor::

    using THREAD_NS::mutex;
    using THREAD_NS::lock_guard;
    using THREAD_NS::unique_lock;
    using THREAD_NS::condition_variable;
    using std::function;
        
    method SerialExecutor() :
        _state(new State())
    {
    }
        
	void method addTask(const function<void(void)> &task)
	{
        bool wasRunning;
        {
            lock_guard<mutex> l(_state->_tasksMutex);
            _state->_tasks.push(task);
            wasRunning = _state->_running;
            if (!wasRunning) {
                _state->_running = true;
            }
        }
        
        if (!wasRunning) {
            auto state = _state;
            ExecutorPool::instance().addTask([state]{
                
                while(true) {
                    MutableTask task;
                    {
                        lock_guard<mutex> l(state->_tasksMutex);
                        if (state->_tasks.empty()) {
                            state->_running = false;
                            break;
                        }
                        task = state->_tasks.front();
                        state->_tasks.pop();
                        state->_threadId = THREAD_NS::this_thread::get_id();
                    }
                    if (task) {
                        task();
                    }
                }
            });
        }
	}

	void method addTaskAndWait(const function<void(void)> &task)
	{
        bool recursive = false;
        {
            lock_guard<mutex> l(_state->_tasksMutex);
            if (_state->_running && _state->_threadId == THREAD_NS::this_thread::get_id()) {
                recursive = true;
            }
        }
        if (recursive) {
            task();
            return;
        }
        
        condition_variable condition;
        bool finished = false;
        mutex m;
        
        addTask([&]{
            task();
            
            lock_guard<mutex> l(m);
            finished = true;
            condition.notify_all();
        });
        
        unique_lock<mutex> l(m);
        while (!finished) {
            condition.wait(l);
        }
	}
    
    method ~SerialExecutor()
    {
    }
    
#undef method
#define method SerialExecutor::State::
    
    method State() :
        _running(false)
    {
    }

#undef method
    
}
