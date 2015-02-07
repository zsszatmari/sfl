//
//  PoolableExecutor.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "Serializer.h"
#include "stdplus.h"


namespace ClientDb
{
    
#define method Serializer::

    using THREAD_NS::mutex;
    using THREAD_NS::lock_guard;
    using THREAD_NS::unique_lock;
    using THREAD_NS::condition_variable;
    using std::unique_ptr;
    using std::function;

    method Serializer() :
        done(0),
        notified(0)
    {
        // this must be the last thing we do in the constructor since threadFunction immediately kicks in
        currentThread = unique_ptr<THREAD_NS::thread>(new THREAD_NS::thread([this](){
            threadFunction();
        }));
    }

    method ~Serializer()
    {
        {
            lock_guard<mutex> lock(tasksMutex);
            notified = true;
            done = true;
            tasksWakeup.notify_one();
        }

        currentThread->join();
    }
    
    void method threadFunction()
    {
        unique_lock<mutex> lock(tasksMutex);
        while (true) {
            while (!notified) {
                tasksWakeup.wait(lock);
            }
            notified = false;
            
            while (tasks.size() > 0) {
                auto task = tasks.front();
                tasks.pop();
                
                lock.unlock();
                // if any exception occurs, it is not handled here
                
                task();
                
                lock.lock();
            }
            
            // only return if every task has finished
            if (done) {
                //cerr << "ret" << done;
                return;
            }
        }
    }
    
    void method addTask(const function<void(void)> &task)
    {
        lock_guard<mutex> lock(tasksMutex);
        tasks.push(task);
        notified = true;
        tasksWakeup.notify_one();
    }
    
    typedef int exception_ptr;
    
    /*
    static void performTask(const function<void(void)> &task, exception_ptr &exception)
    {
        try {
                task();
            } catch (...) {
#ifdef USE_BOOST_INSTEAD_OF_CPP11
                exception = boost::current_exception();
#else
#ifndef NO_EXCEPTION_PTR
                exception = std::current_exception();
#endif
#endif
            }
    }*/
    
    void method addTaskAndWait(const function<void(void)> &task)
    {
        if (currentThread->get_id() == THREAD_NS::this_thread::get_id()) {
            task();
            return;
        }
        
        bool finish = false;
        condition_variable finishWakeup;
        mutex finishMutex;
    
        exception_ptr exception;
        
        this->addTask([&]{
            //performTask(task, exception);
            task();
            lock_guard<mutex> lock(finishMutex);
            finish = true;
            finishWakeup.notify_one();
        });
        
        unique_lock<mutex> lock(finishMutex);
        while (!finish) {
            finishWakeup.wait(lock);
        }

/*
#ifndef NO_EXCEPTION_PTR
        if (exception) {
#ifdef USE_BOOST_INSTEAD_OF_CPP11
            boost::rethrow_exception(exception);
#else
            std::rethrow_exception(exception);
#endif
        }
#endif*/
    }
}
