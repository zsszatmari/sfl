//
//  MainExecutor.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/14/13.
//
//

#ifndef __G_Ear_Player__MainExecutor__
#define __G_Ear_Player__MainExecutor__

#include "IExecutor.h"

namespace Base
{
    class MainExecutor final : public IExecutor
    {
    public:
        virtual void addTask(Task &task);
        virtual void addTaskAndWait(Task &task);
        
        static MainExecutor &instance();

    private:
        MainExecutor();
    };
}

#endif /* defined(__G_Ear_Player__MainExecutor__) */
