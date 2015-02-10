//
//  ManagedValue.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/11/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ManagedValue__
#define __G_Ear_core__ManagedValue__

#include "ValueSignal.h"
#include "stdplus.h"

#ifdef DEBUG
//#define DEBUGHOOK
#endif

namespace Base
{
    void ManagedValueDebug();
    
    template<typename T>
    class ManagedValue
    {
    public:
        ManagedValue();
        ManagedValue(T t);
        T operator=(T t);
        operator T() const;
        T get() const;
        
        ValueConnector<T> connector();
        
// ---
#ifdef DEBUGHOOK
        void setDebugHook(const function<void()> &f);
#endif
        
    private:
        ManagedValue(const ManagedValue<T> &) = delete;
        ManagedValue<T> &operator=(const ManagedValue<T> &) = delete;
        
        ValueSignal<T> _signal;
#ifdef DEBUGHOOK
        function<void()> _debugHook;
#endif
    };
    
    
    
    template<typename T>
    ManagedValue<T>::ManagedValue()
    {
    }
    
    template<typename T>
    ManagedValue<T>::ManagedValue(T t)
    {
        _signal.signal(t);
    }
    
    template<typename T>
    T ManagedValue<T>::operator=(T t)
    {
#ifdef DEBUGHOOK
        if (_debugHook) {
            ManagedValueDebug();
            _debugHook();
        }
#endif
#ifdef __clang__
        // disable optimization because of a bug in clang...
        std::vector<int> v;
#endif
        _signal.signal(t);
        return t;
    }
    
#ifdef DEBUGHOOK
    template<typename T>
    void ManagedValue<T>::setDebugHook(const function<void()> &f)
    {
        _debugHook = f;
    }
#endif
    
    template<typename T>
    ValueConnector<T> ManagedValue<T>::connector()
    {
        return _signal.connector();
    }
    
    template<typename T>
    ManagedValue<T>::operator T() const
    {
        return _signal.lastValue();
    }
    
    template<typename T>
    T ManagedValue<T>::get() const
    {
        return _signal.lastValue();
    }
}

#endif /* defined(__G_Ear_core__ManagedValue__) */
