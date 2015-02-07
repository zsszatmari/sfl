//
//  ValidPtr.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 09/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ValidPtr__
#define __G_Ear_core__ValidPtr__

#include <cstddef>
#include <assert.h>
#include "stdplus.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    template<class T>
    class AtomicPtr;
    
    template<class T>
    class ValidPtr
    {
    public:
        ValidPtr();
        T &operator*() const;
        T *operator->() const;
    
        ValidPtr<T> &operator=(const shared_ptr<T> &t);
        ValidPtr(const shared_ptr<T> &t);
        explicit ValidPtr(T *t);
        explicit ValidPtr(const AtomicPtr<T> &);
        operator shared_ptr<T>() const;
        shared_ptr<T> get() const;
        operator ValidPtr<const T>() const;
        
    private:
        shared_ptr<T> _ptr;
        explicit ValidPtr(std::nullptr_t t); // delete
    };
    
    
#define def template<class T>
#define method ValidPtr<T>::
    
    def
    method ValidPtr() :
        _ptr(new T())
    {
    }
    
    def
    method ValidPtr(T *t) :
        _ptr(t)
    {
        assert(t);
    }
    
    def
    ValidPtr<T> & method operator=(const shared_ptr<T> &t)
    {
        _ptr = t;
        assert(t);
        return *this;
    }

    def
    T & method operator*() const
    {
        return _ptr.operator*();
    }
    
    def
    T * method operator->() const
    {
        return _ptr.operator->();
    }
    
    def
    method operator shared_ptr<T>() const
    {
        return _ptr;
    }
    
    def
    shared_ptr<T> method get() const
    {
        return _ptr;
    }
    
    def
    method operator ValidPtr<const T>() const
    {
        return ValidPtr<const T>(_ptr);
    }

    def
    method ValidPtr(const shared_ptr<T> &t) :
        _ptr(t)
    {
        assert(_ptr);
    }
    
    def
    method ValidPtr(const AtomicPtr<T> &rhs) :
        _ptr(rhs)
    {
        assert((bool)_ptr);
    }
    
#undef method
#undef def
}

#endif /* defined(__G_Ear_core__ValidPtr__) */
