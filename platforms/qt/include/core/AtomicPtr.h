//
//  ImmutablePtr.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 07/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ImmutablePtr__
#define __G_Ear_core__ImmutablePtr__

#include <functional>
#include "stdplus.h"
#include "ValidPtr.h"
#include MEMORY_H
#include ATOMIC_H

/*
#ifdef __APPLE__
// ios 6's libc++ is not so good
#define FAULTY_ATOMIC_SP
#include <boost/atomic.hpp>
#include "spinlock.h"
#endif
*/
#ifdef __MINGW32__
#define FAULTY_ATOMIC_SP

#include MUTEX_H
using THREAD_NS::mutex;
using THREAD_NS::lock_guard;
#endif

namespace Gear
{
    // the object pointed to is guaranteed to be immutable
    // but changing it is atomic
    
    template<class T>
    class AtomicPtr
    {
    public:
        AtomicPtr();
        AtomicPtr(const T &rhs);
        AtomicPtr(std::unique_ptr<T> &rhs);
        AtomicPtr(const shared_ptr<const T> &rhs);
        AtomicPtr(const AtomicPtr<T> &);
        
        // these are wrong: there is no guarantee that the returned naked pointer will survive. we will have to return a shared_ptr
        //T& operator*() const;
        //T* operator->() const;
        
        AtomicPtr<T> &operator=(const T &);
        //AtomicPtr<T> &operator=(const shared_ptr<T> &);
        
        // it's crucial that there are not other non-const references to rhs's object!
        AtomicPtr<T> &operator=(std::unique_ptr<T> &rhs);
        AtomicPtr<T> &operator=(const AtomicPtr<T> &);
        shared_ptr<const T> operator->() const;
        shared_ptr<const T> get() const;
        shared_ptr<const T> ptr() const;
        void set(const std::function<void(T &)> &);
        void setC(const std::function<T(const T &)> &);
        void setS(const std::function<shared_ptr<const T>(shared_ptr<T> &)> &);
        operator shared_ptr<const T>() const;
        operator T() const;
        operator ValidPtr<const T>() const;
        void reset();
        
        typedef T element_type;
        
    private:
        //AtomicPtr(const shared_ptr<T> &rhs);
        
        shared_ptr<const T> _ptr;
#ifdef FAULTY_ATOMIC_SP
        //mutable Base::spinlock _ptrMutex;
        mutable mutex _ptrMutex;
#endif
        
        shared_ptr<const T> load() const
        {
#ifndef FAULTY_ATOMIC_SP
            return ATOMIC_NS::atomic_load(&_ptr);
#else
            lock_guard<decltype(_ptrMutex)> l(_ptrMutex);
            return _ptr;
#endif
        }
        
        void store(const shared_ptr<const T> &newValue)
        {
#ifndef FAULTY_ATOMIC_SP
            ATOMIC_NS::atomic_store(&_ptr, newValue);
#else
            lock_guard<decltype(_ptrMutex)> l(_ptrMutex);
            _ptr = newValue;
#endif
        }
        
        bool compare_exchange_strong(shared_ptr<const T> *expected, const shared_ptr<const T> &desired)
        {
#ifndef FAULTY_ATOMIC_SP
            return ATOMIC_NS::atomic_compare_exchange_strong(&_ptr, expected, desired);
#else
            lock_guard<decltype(_ptrMutex)> l(_ptrMutex);
            if (_ptr == *expected) {
                _ptr = desired;
                return true;
            } else {
                return false;
            }
#endif
        }
        
        template<class U>
        friend class AtomicPtr;
    };
    
    template<class T>
    class SharedAtomicPtr
    {
    public:
        SharedAtomicPtr();
    };
    
#define def template<class T>
#define method AtomicPtr<T>::
    
    def
    method AtomicPtr() :
        _ptr(new T())
    {
    }
    
    def
    method AtomicPtr(const T &rhs) :
        _ptr(new T(rhs))
    {
    }

    def
    shared_ptr<const T> method operator->() const
    {
        return load();
    }
    
    def
    shared_ptr<const T> method ptr() const
    {
        return load();
    }
    
    def
    shared_ptr<const T> method get() const
    {
        return load();
    }
    
    def
    method operator T() const
    {
        auto p = ptr();
        return T(*p);
    }
    
    
    def
    void method set(const std::function<void(T &)> &f)
    {
        while (true) {
            shared_ptr<const T> original = load();
            
            shared_ptr<T> copiedPtr(new T(*original));
            f(*copiedPtr);
            shared_ptr<const T> constPtr(copiedPtr);
            
            if (compare_exchange_strong(&original, constPtr)) {
                break;
            }
        }
    }
    
    def
    void method setC(const std::function<T(const T &)> &f)
    {
        while (true) {
            shared_ptr<const T> original = load();
            shared_ptr<const T> constPtr(new T(f(*original)));
            
            if (compare_exchange_strong(&original, constPtr)) {
                break;
            }
        }
    }
    
    def
    void method setS(const std::function<shared_ptr<const T>(shared_ptr<T> &)> &f)
    {
        // we give a non-const value for in-place manipulation
        // (this is for making the least possible number of copies)
        while (true) {
            shared_ptr<const T> original = load();
            
            shared_ptr<T> copiedPtr(new T(*original));
            auto result = f(copiedPtr);
            if (!result) {
                // nullptr means the caller changed his/her mind
                break;
            }
            
            if (compare_exchange_strong(&original, result)) {
                break;
            }
        }
    }
    
    def
    method operator shared_ptr<const T>() const
    {
        return load();
    }
    
    def
    method operator ValidPtr<const T>() const
    {
        AtomicPtr<const T> intermediate(load());
        return ValidPtr<const T>(intermediate);
    }
    
    def
    AtomicPtr<T> & method operator=(const T &rhs)
    {
        store(shared_ptr<const T>(new T(rhs)));
        return *this;
    }
    
    def
    AtomicPtr<T> & method operator=(const AtomicPtr<T> &rhs)
    {
        shared_ptr<const T> rhsPtr = rhs;
        store(rhsPtr);
        return *this;
    }
    
    def
    method AtomicPtr(const AtomicPtr<T> &rhs)
    {
        shared_ptr<const T> rhsPtr = rhs;
        store(rhsPtr);
    }
    
    def
    method AtomicPtr(std::unique_ptr<T> &rhs)
    {
        shared_ptr<const T> constRhs(rhs.release());
        store(constRhs);
    }
    
    /*def
    method AtomicPtr(const shared_ptr<T> &rhs)
    {
        store(rhs);
    }*/
    
    def
    method AtomicPtr(const shared_ptr<const T> &rhs)
    {
        store(rhs);
    }
    
    
    def
    AtomicPtr<T> & method operator=(std::unique_ptr<T> &rhs)
    {
        shared_ptr<const T> constRhs(rhs.release());
        store(constRhs);
        return *this;
    }
    
    def
    void method reset()
    {
        shared_ptr<const T> zero(new T());
        store(zero);
    }
    
#undef method
#undef def
}

#endif /* defined(__G_Ear_core__ImmutablePtr__) */
