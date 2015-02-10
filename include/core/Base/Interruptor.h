//
//  Interruptor.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/18/13.
//
//

#ifndef __G_Ear_Player__Interruptor__
#define __G_Ear_Player__Interruptor__

#include "stdplus.h"
#include CONDITION_VARIABLE_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Base
{
    class Interruptor final
    {
    public:
        class Handle
        {
        public:
            void interrupt();
            
            THREAD_NS::mutex _mutex;
            THREAD_NS::condition_variable _condition;
            bool _interrupted;
            
        private:
            Handle();
            
            friend class Interruptor;
        };
        
        Interruptor();
        ~Interruptor();
        
        static shared_ptr<Handle> getHandle();
        
    private:
        Interruptor(const Interruptor &rhs); // delete
        Interruptor &operator=(const Interruptor &rhs); // delete
        
        shared_ptr<Handle> _handle;
    };
}

#endif /* defined(__G_Ear_Player__Interruptor__) */
