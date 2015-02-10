//
//  DefaultExecutor.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/14/13.
//
//

#ifndef __G_Ear_Player__DefaultExecutor__
#define __G_Ear_Player__DefaultExecutor__

#include "stdplus.h"
#include MEMORY_H
#include "IExecutor.h"

namespace Base
{
    class core_export DefaultExecutor
    {
    public:
        static MEMORY_NS::shared_ptr<IExecutor> &instance();
        static void registerInstance(const MEMORY_NS::shared_ptr<IExecutor> &rhs);
        
    private:
        DefaultExecutor(); // delete
    };
}

#endif /* defined(__G_Ear_Player__DefaultExecutor__) */
