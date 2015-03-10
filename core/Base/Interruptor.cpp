//
//  Interruptor.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/18/13.
//
//

#include "Interruptor.h"
#include "Thread.h"

namespace Base
{
#define method Interruptor::
    
    // thread_local shared_ptr<Interruptor::Handle> _threadHandle;
    
#ifdef USE_PTHREADS
    static pthread_key_t threadKey;
    static void makeKey()
    {
        (void) pthread_key_create(&threadKey, NULL);
        pthread_setspecific(threadKey, nullptr);
    }
    
    static void threadLocalInit()
    {
        static pthread_once_t key_once = PTHREAD_ONCE_INIT;
        (void) pthread_once(&key_once, &makeKey);
    }
    
    static shared_ptr<Interruptor::Handle> &getThreadLocalHandle()
    {
        threadLocalInit();
        void *ptr = pthread_getspecific(threadKey);
        if (ptr == nullptr) {
            static shared_ptr<Interruptor::Handle> nullRet;
            return nullRet;
        }
        return *(reinterpret_cast<shared_ptr<Interruptor::Handle> *>(ptr));
    }
    
    static void setThreadLocalHandle(const shared_ptr<Interruptor::Handle> &handle)
    {
        threadLocalInit();
        void *ptr = pthread_getspecific(threadKey);
        if (ptr == nullptr) {
            shared_ptr<Interruptor::Handle> *ptrHandle = new shared_ptr<Interruptor::Handle>();
            ptr = ptrHandle;
            pthread_setspecific(threadKey, ptrHandle);
        }
        shared_ptr<Interruptor::Handle> *ptrHandle = reinterpret_cast<shared_ptr<Interruptor::Handle> *>(ptr);
        shared_ptr<Interruptor::Handle> &s = *ptrHandle;
        s = handle;
    }

#else

	static __declspec(thread) void *ptr;

	static shared_ptr<Interruptor::Handle> &getThreadLocalHandle()
	{
		if (ptr == nullptr) {
			static shared_ptr<Interruptor::Handle> nullRet;
			return nullRet;
		}
		return *(reinterpret_cast<shared_ptr<Interruptor::Handle> *>(ptr));
	}

	static void setThreadLocalHandle(const shared_ptr<Interruptor::Handle> &handle)
	{
		if (ptr == nullptr) {
			shared_ptr<Interruptor::Handle> *ptrHandle = new shared_ptr<Interruptor::Handle>();
			ptr = ptrHandle;
		}
		shared_ptr<Interruptor::Handle> *ptrHandle = reinterpret_cast<shared_ptr<Interruptor::Handle> *>(ptr);
		shared_ptr<Interruptor::Handle> &s = *ptrHandle;
		s = handle;
	}

#endif
    
    method Interruptor() :
        _handle(new Interruptor::Handle())
    {
        setThreadLocalHandle(_handle);
    }
    
    method ~Interruptor()
    {
        setThreadLocalHandle(shared_ptr<Interruptor::Handle>());
    }
    
    shared_ptr<Interruptor::Handle> method getHandle()
    {
        auto ret = getThreadLocalHandle();
        if (ret) {
            return ret;
        }
        return shared_ptr<Interruptor::Handle>(new Interruptor::Handle());
    }
    
#undef method
#define method Interruptor::Handle::
    
    method Handle() :
        _interrupted(false)
    {
    }
    
    void method interrupt()
    {
        THREAD_NS::lock_guard<THREAD_NS::mutex> l(_mutex);
        _interrupted = true;
        _condition.notify_all();
    }
}
