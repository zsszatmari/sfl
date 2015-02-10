//
//  AtomicMutablePtr.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 18/04/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__AtomicMutablePtr__
#define __G_Ear_core__AtomicMutablePtr__

#ifdef __MINGW32__
#define FAULTY_ATOMIC_SP

#include MUTEX_H
using THREAD_NS::mutex;
using THREAD_NS::lock_guard;
#endif

namespace Gear
{
    template<class T>
    class AtomicMutablePtr
    {
    public:
        AtomicMutablePtr &operator=(const shared_ptr<T> &value);
        operator shared_ptr<T>() const;
        
    private:
        shared_ptr<T> _ptr;
#ifdef FAULTY_ATOMIC_SP
        mutable mutex _ptrMutex;
#endif
    };
    
#define def template<class T>
#define method AtomicMutablePtr<T>::
    
    def
    AtomicMutablePtr<T> & method operator=(const shared_ptr<T> &value)
    {
#ifndef FAULTY_ATOMIC_SP
        ATOMIC_NS::atomic_store(&_ptr, value);
#else
        lock_guard<decltype(_ptrMutex)> l(_ptrMutex);
        _ptr = value;
#endif
        return *this;
    }
    
    def
    method operator shared_ptr<T>() const
    {
#ifndef FAULTY_ATOMIC_SP
        return ATOMIC_NS::atomic_load(&_ptr);
#else
        lock_guard<decltype(_ptrMutex)> l(_ptrMutex);
        return _ptr;
#endif
    }
    
#undef method
#undef def
}

#endif /* defined(__G_Ear_core__AtomicMutablePtr__) */
