//
//  Connector.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__Connector__
#define __G_Ear_iOS__Connector__

#include "SignalConnection.h"

namespace Base
{    
    template<class Signal>
    class Connector
    {
    public:
        // copying is legal but take care... don't let it live longer than the signal source (don't store it)
        //Connector(const Connector &rhs);
        
        // synopsis:
        // 1. connect a class method
        // void connect(void(B::*method)(), const shared_ptr<B> sp);
        // 2. connect an arbitrary function wrapper. Must keep the returned EventConnection alive!
        //EventConnection connect(const function<void()> &f);
        
        template<typename A, typename B>
        SignalConnection connectWithExecutor(A a, B b);
        
        template<typename A, typename B>
        void connect(A a, B b);
        
        template<typename A>
        SignalConnection connect(A);
        
        template<typename A, typename B, typename C>
        void connect(A a, B b, C c);
    
#ifndef NO_FRIEND_TEMPLATE
    protected:
#endif
        Connector(Signal &signal);
        
    private:
        Connector &operator=(const Connector &rhs); // delete
        
        Signal &_signal;
        
#ifndef NO_FRIEND_TEMPLATE
        friend Signal;
#endif
    };
    
    
    template<class Signal>
    Connector<Signal>::Connector(Signal &signal) :
        _signal(signal)
    {
    }
    
    template<class Signal>
    template<typename A, typename B>
    void Connector<Signal>::connect(A a, B b)
    {
        _signal.connect(a, b);
    }
    
    template<class Signal>
    template<typename A>
    SignalConnection Connector<Signal>::connect(A a)
    {
        return _signal.connect(a);
    }
    
    template<class Signal>
    template<typename A, typename B>
    SignalConnection Connector<Signal>::connectWithExecutor(A a, B b)
    {
        return _signal.connect(a, b);
    }
    
    template<class Signal>
    template<typename A, typename B, typename C>
    void Connector<Signal>::connect(A a, B b, C c)
    {
        _signal.connect(a, b, c);
    }
}

#endif /* defined(__G_Ear_iOS__Connector__) */
