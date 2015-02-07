//
//  SignalConnection.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "SignalConnection.h"

namespace Base
{
#define method SignalConnectionTokenBase::

    using std::function;
    
    method SignalConnectionTokenBase()
    {
    }
    
    method ~SignalConnectionTokenBase()
    {
    }
    
#undef method
    
#define method SignalConnectionToken::
    
    method SignalConnectionToken(const function<void(void)> &f) :
        _f(f)
    {
    }
        
    void method signal()
    {
        _f();
    }
}