//
//  Thread.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/21/13.
//
//

#ifndef __G_Ear__Thread__
#define __G_Ear__Thread__

#include "stdplus.h"
#ifndef _WIN32
#define USE_PTHREADS
#include <pthread.h>
#else
#include THREAD_H
#endif
#include <functional>

namespace Base
{
    using std::function;

    // boost::thread triggers serious bug in clang, c++11 is not available, so we have to supply ours...
    class Thread final
    {
    public:
        Thread(const std::function<void()> f);
        ~Thread();
        void join();
        void detach();
        
        bool isCurrent();
        
    private:
        Thread(); // delete
        Thread(const Thread &rhs); // delete
        Thread &operator=(const Thread &rhs); // delete
        
        static void *threadfunc(void *param);
        const std::function<void()> _f;
#ifndef USE_PTHREADS
		THREAD_NS::thread thr;
#else
        pthread_t thr;
#endif
        bool joinable;
    };
    
    
    
    // it seems that it wasn't boost's mutex implementation to blame, so we are safe to use it, but we keep our own just in case
	/*
	class Mutex final
    {
    public:
        Mutex();
        ~Mutex();
        void lock();
        void unlock();
        
    private:
        Mutex(const Thread &rhs); // delete
        Mutex &operator=(const Thread &rhs); // delete
        
        pthread_mutex_t _mutex;
    };*/
    
    template<class MutexClass>
    class Lock final
    {
    public:
        Lock(MutexClass &m);
        ~Lock();
        void unlock();
        
    private:
        MutexClass &_mutex;
        bool locked;
        
        Lock(const Lock &rhs); // delete
        Lock &operator=(const Lock &rhs); //delete
    };
    
#define templatedef template<class MutexClass>
#define method Lock<MutexClass>::
    
    templatedef
    method Lock(MutexClass &m) : _mutex(m), locked(false)
    {
        m.lock();
        locked = true;
    }
    
    templatedef
    method ~Lock()
    {
        unlock();
    }
    
    templatedef
    void method unlock()
    {
        if (locked) {
            _mutex.unlock();
        }
    }
    
#undef templatedef
#undef method
}

#endif /* defined(__G_Ear__Thread__) */
