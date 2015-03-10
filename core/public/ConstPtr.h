//
//  ImmutablePtr.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 07/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef G_Ear_core_ImmutablePtr_h
#define G_Ear_core_ImmutablePtr_h

#include "stdplus.h"

namespace Gear
{
    template<class T>
    class ConstPtr
    {
    public:
        ConstPtr();
        ConstPtr(const ConstPtr &);
        operator shared_ptr<T>();
        operator shared_ptr<const T>() const;
        shared_ptr<T> operator->() const;
        
    private:
        ConstPtr &operator=(const ConstPtr &); // delete
        
        const shared_ptr<T> _ptr;
    };
    
    
#define def template<class T>
#define method ConstPtr<T>::
    
    def
    method ConstPtr() :
        _ptr(new T())
    {
    }
    
    def
    method ConstPtr(const ConstPtr &rhs) :
        _ptr(rhs._ptr)
    {
    }
    
    def
    method operator shared_ptr<T>()
    {
        return _ptr;
    }
    
    def
    method operator shared_ptr<const T>() const
    {
        return _ptr;
    }
    
    def
    shared_ptr<T> method operator->() const
    {
        return _ptr;
    }
    
#undef method
#undef def

}
    
#endif
