//
//  EventSignal.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "EventSignal.h"

namespace Base
{    
#define method EventSignal::

    using std::function;
    
    method EventSignal()
    {
    }
    
    void method signal()
    {
        _signal();
    }
    
    SignalConnection method connect(const function<void()> &f, const shared_ptr<IExecutor> &executor)
    {
        shared_ptr<SignalConnectionToken> connection(new SignalConnectionToken(f));
        connect(&SignalConnectionToken::signal, connection, executor);
        return connection;
    }
    
    Connector<EventSignal> method connector()
    {
        return Connector<EventSignal>(*this);
    }
    
#undef method

}