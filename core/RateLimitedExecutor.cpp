//
//  RateLimitedExecutor.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/23/13.
//
//

#include "RateLimitedExecutor.h"

namespace Gear
{
#define method RateLimitedExecutor::
    
    using namespace Base;
    using THREAD_NS::this_thread::sleep_for;
    
    method RateLimitedExecutor(float limit) :
        _limitMillisec(limit*1000)
    {
    }
    
    void method addTask(IExecutor::Task &task)
    {
        SerialExecutor::addTask([=]{
            
            auto now = CHRONO_NS::steady_clock::now();
            task();
            
            auto elapsedMilli = CHRONO_NS::duration_cast<CHRONO_NS::milliseconds>((now - CHRONO_NS::steady_clock::now())).count();
            if (elapsedMilli < _limitMillisec) {
                CHRONO_NS::milliseconds w(_limitMillisec - elapsedMilli);
                sleep_for(w);
            }
        });
    }
    
    void method addTaskAndWait(IExecutor::Task &task)
    {
        auto limit = _limitMillisec;
        SerialExecutor::addTaskAndWait([&task, limit, this]{
            auto now = CHRONO_NS::steady_clock::now();
            task();
            
            auto elapsedMilli = CHRONO_NS::duration_cast<CHRONO_NS::milliseconds>((now - CHRONO_NS::steady_clock::now())).count();
            if (elapsedMilli < limit) {
                
                CHRONO_NS::milliseconds w(limit - elapsedMilli);
                this->SerialExecutor::addTask([w]{
                    sleep_for(w);
                });
            }
        });
    }
}
