//
//  Thread.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/21/13.
//
//

#include <stdexcept>
#include "Thread.h"


namespace Base
{
    
#define method Thread::
	
    void * method threadfunc(void *parm)
    {
        typedef function<void()> FType;
        function<void()> *function = reinterpret_cast<FType *>(parm);
        (*function)();
        delete function;
        return NULL;
    }
    
    method Thread(const function<void()> f) : _f(f), joinable(0)
#ifdef USE_PTHREADS
		,thr(0)
#endif
    {
		function<void()> *copiedFunction = new function<void()>(f);
        
#ifndef USE_PTHREADS
		thr = THREAD_NS::thread(threadfunc, copiedFunction);
#else
        int ret = pthread_create(&thr, NULL, &threadfunc, copiedFunction);
        if (ret != 0) {
            throw new std::runtime_error("error creating thread");
        }
#endif
        joinable = true;
    }
    
    method ~Thread()
    {
        if (joinable) {
            std::terminate();
        }
    }
    
    bool method isCurrent()
    {
#ifdef USE_PTHREADS
        return (pthread_self() == thr);
#else
		return THREAD_NS::this_thread::get_id() == thr.get_id();
#endif
    }
    
    void method join()
    {
        if (joinable) {
#ifdef USE_PTHREADS
            pthread_join(thr, NULL);
#else
			thr.join();
#endif
            joinable = false;
        } else {
            throw new std::runtime_error("not joinable");
        }
    }
    
    void method detach()
    {
#ifndef USE_PTHREADS
		thr.detach();
#endif
        joinable = false;
    }
    
#undef method

	/*
#define method Mutex::
    method Mutex() : _mutex(PTHREAD_MUTEX_INITIALIZER)
    {
    }
    
    method ~Mutex()
    {
    }
    
    void method lock()
    {
        //std::cout << "will lock\n";
        pthread_mutex_lock(&_mutex);
        //std::cout << "locked\n";
    }
    
    void method unlock()
    {
        pthread_mutex_unlock(&_mutex);
        //std::cout << "unlocked\n";
    }
    
#undef method*/
}