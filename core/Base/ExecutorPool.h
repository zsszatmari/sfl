//
//  ExecutorPool.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ExecutorPool__
#define __G_Ear_core__ExecutorPool__

#include "stdplus.h"
#include MEMORY_H
#include "PoolableExecutor.h"

namespace Base
{
    class ExecutorPool final : public IExecutor
    {
    public:
        static ExecutorPool &instance();
        
        virtual void addTask(Task &task);
        virtual void addTaskAndWait(Task &task);
        
    private:
        ExecutorPool();
        
        THREAD_NS::mutex _idleMutex;
        std::vector<MEMORY_NS::shared_ptr<PoolableExecutor>> _idle;
    };
}

#endif /* defined(__G_Ear_core__ExecutorPool__) */
