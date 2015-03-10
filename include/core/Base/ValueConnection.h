//
//  ValueConnection.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/11/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef G_Ear_core_ValueConnection_h
#define G_Ear_core_ValueConnection_h

#include "SignalConnection.h"
#include <functional>

namespace Base
{
    template<class ValueType>
    class ValueConnectionToken : public SignalConnectionTokenBase
    {
    public:
        ValueConnectionToken(const std::function<void(const ValueType value)> &f);
        void signal(const ValueType value);
        
    private:
        std::function<void(const ValueType value)> _f;
    };
    
    
    
    
    template<class ValueType>
    ValueConnectionToken<ValueType>::ValueConnectionToken(const std::function<void(const ValueType value)> &f) :
        _f(f)
    {
    }
    
    template<class ValueType>
    void ValueConnectionToken<ValueType>::signal(const ValueType value)
    {
        _f(value);
    }
}

#endif
