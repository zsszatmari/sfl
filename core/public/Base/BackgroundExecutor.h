//
//  BackgroundExecutor.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__BackgroundExecutor__
#define __G_Ear_Player__BackgroundExecutor__

#include "IExecutor.h"

namespace Base
{
    class core_export BackgroundExecutor final : public IExecutor
    {
    public:
        virtual void addTask(Task &task);
        virtual void addTaskAndWait(Task &task);
        
        static BackgroundExecutor &instance();
        
    private:
        BackgroundExecutor();
    };
}

#endif /* defined(__G_Ear_Player__BackgroundExecutor__) */
