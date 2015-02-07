//
//  IAsyncData.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/20/13.
//
//

#include <algorithm>
#include <cassert>
#include "IAsyncData.h"
#include "Thread.h"
#include "Interruptor.h"
#include "stdplus.h"

namespace Gear {
    
    using std::make_pair;
    using THREAD_NS::lock_guard;
    using THREAD_NS::mutex;
    using THREAD_NS::unique_lock;
    using std::function;

#define method IAsyncData::
    method IAsyncData()
    {
    }

    method ~IAsyncData()
    {
    }
    
    void method waitAsync(int offset, const function<void()> &f)
    {
        assert(offset >= 0);
        
        bool shouldCall = false;
        int avail;
        {
            lock_guard<mutex> l(waitingMutex);
            avail = offsetAvailable(offset);
            if (avail > 0 || finished()) {
                shouldCall = true;
            } else {
                waiting.push_back(make_pair(offset, f));
            }
        }
        // lock is not re-entrant so we mustn't be locking!
        if (shouldCall) {
            f();
        } else {
            signal();
        }
    }
    
    void method waitSync(int offset)
    {
        auto interruptor = Base::Interruptor::getHandle();
        shared_ptr<bool> notified = shared_ptr<bool>(new bool(false));
        waitAsync(offset, [interruptor, notified]{
            
            lock_guard<mutex> lockNotify(interruptor->_mutex);
            *notified = true;
            interruptor->_condition.notify_all();
        });

        unique_lock<mutex> lockWait(interruptor->_mutex);
        while (!*notified && !interruptor->_interrupted) {
        	// general 30 sec timeout
        	CHRONO_NS::milliseconds w(30*1000);
            bool timeouted = (interruptor->_condition.wait_for(lockWait, w)) == THREAD_NS::cv_status::timeout;
            if (timeouted) {
                return;
            }
        }
    }
    
    void method signal()
    {
        vector<pair<int, function<void()>>> toCall;
        {
            lock_guard<mutex> l(waitingMutex);
            auto removedIterator = std::partition(waiting.begin(), waiting.end(), [&](pair<int, function<void()>> &p){
                
                int offset = p.first;
                int avail = offsetAvailable(offset);
                
                bool finish = finished();
                
                //cout << avail << "," << avail0 << "," << offset << "," << finish << endl;
                return !(avail > 0 || finish);
            });
            toCall.insert(toCall.end(), removedIterator, waiting.end());
            waiting.erase(removedIterator, waiting.end());
        }
        
        for (auto it = toCall.begin() ; it != toCall.end() ; ++it) {
        	auto &p = *it;
        	auto &f = p.second;
        	if (f) {
        		f();
        	}
        }
    }
}
