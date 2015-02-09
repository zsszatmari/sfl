//
//  IExecutor.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/14/13.
//
//

#ifndef __G_Ear_Player__IExecutor__
#define __G_Ear_Player__IExecutor__

#include "Environment.h"
#include "stdplus.h"
#include <functional>

namespace Base
{
    class core_export IExecutor
    {
    public:
    	typedef std::function<void(void)> MutableTask;
        typedef const std::function<void(void)> Task;
        
        virtual ~IExecutor();

        virtual void addTask(Task &task) = 0;
        virtual void addTaskAndWait(Task &task) = 0;
    };
}

#endif /* defined(__G_Ear_Player__IExecutor__) */
