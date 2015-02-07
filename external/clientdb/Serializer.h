//
//  PoolableExecutor.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef CLIENTDB_SERIALIZER
#define CLIENTDB_SERIALIZER

#include "stdplus.h"
#include <queue>
#include <functional>
#include <memory>
#include <condition_variable>
#include THREAD_H
#include MUTEX_H

namespace ClientDb
{
    using std::queue;
    
    class Serializer final
    {
    public:
        Serializer();
        ~Serializer();
        
        virtual void addTask(const std::function<void()> &task);
        virtual void addTaskAndWait(const std::function<void()> &task);
        
    private:
        Serializer(const Serializer &rhs);
        const Serializer &operator=(const Serializer &rhs);
        
        //void addTaskInt(const function<int(void)> &task);
        //void addTaskIntAndWait(const function<int(void)> &task);
        
        std::unique_ptr<THREAD_NS::thread> currentThread;
        queue<std::function<void(void)>> tasks;
        THREAD_NS::mutex tasksMutex;
        THREAD_NS::condition_variable tasksWakeup;
        volatile bool done;
        volatile bool notified;
        
        void threadFunction();
    };
    
}


#endif /* defined(__G_Ear_core__PoolableExecutor__) */
