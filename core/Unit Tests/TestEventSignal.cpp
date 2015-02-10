//
//  TestEventSignal.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "tut/tut.hpp"
#include "EventSignal.h"
#include "InstantExecutor.h"

using namespace Base;

namespace tut
{
    struct eventinfo
    {
    };
    
    typedef test_group<eventinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("EventSignal");
    
    class SignalTestClass final
    {
    public:
        SignalTestClass() :
            _signalled(false)
        {
        }
        
        void test()
        {
            _signalled = true;
        }
        
        bool _signalled;
    };
    
    template<>
    template<>
    void testobject::test<1>()
    {
        EventSignal s;
        shared_ptr<SignalTestClass> slot(new SignalTestClass());
        
        s.connect(&SignalTestClass::test, slot, shared_ptr<IExecutor>(new InstantExecutor()));
        ensure("before", slot->_signalled == false);
        
        s.signal();
        
        ensure("after", slot->_signalled == true);
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        EventSignal s;
        weak_ptr<SignalTestClass> weak;
        {
            shared_ptr<SignalTestClass> slot(new SignalTestClass());
            weak = slot;
            
            s.connect(&SignalTestClass::test, slot, shared_ptr<IExecutor>(new InstantExecutor()));
            ensure("weakbefore", weak.use_count() == 1);
        }
        
        ensure("weakafter", weak.use_count() == 0);
    }
    
    template<>
    template<>
    void testobject::test<3>()
    {
        EventSignal s;
        
        int signalCount = 0;
        
        auto f = [&signalCount]{
            ++signalCount;
        };
        
        // using it the wrong way: not keeping the connection:
        s.connect(f, shared_ptr<IExecutor>(new InstantExecutor()));
        s.signal();
        ensure_equals("wrong", signalCount, 0);
        
        {
            auto conn = s.connect(f, shared_ptr<IExecutor>(new InstantExecutor()));
            s.signal();
            ensure_equals("right", signalCount, 1);
        }
        
        s.signal();
        ensure_equals("released", signalCount, 1);
    }
    
    template<>
    template<>
    void testobject::test<4>()
    {
        EventSignal s;
        shared_ptr<SignalTestClass> slot(new SignalTestClass());
        
        //auto shouldnotcompile = s.connector();
        
        s.connector().connect(&SignalTestClass::test, slot, shared_ptr<IExecutor>(new InstantExecutor()));
        ensure("before", slot->_signalled == false);
        
        s.signal();
        
        ensure("after", slot->_signalled == true);

    }
}
