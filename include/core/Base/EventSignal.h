//
//  EventSignal.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__Signal__
#define __G_Ear_iOS__Signal__

#include "stdplus.h"
#include "Environment.h"

#ifdef USE_BOOST_INSTEAD_OF_CPP11
#define BOOST_NO_CXX11_VARIADIC_TEMPLATES
#define BOOST_NO_CXX11_RVALUE_REFERENCES
#endif
#include "boost/signals2.hpp"
#include "Connector.h"
#include "SignalConnection.h"
#include "DefaultExecutor.h"
#include WEAK_H
using MEMORY_NS::shared_ptr;
using MEMORY_NS::weak_ptr;

namespace Base
{
    class core_export EventSignal final
    {
    public:
        EventSignal();
        
        template<class B>
        void connect(void(B::*method)(), const shared_ptr<B> sp, const shared_ptr<IExecutor> &executor = DefaultExecutor::instance());
        
        // must keep the returned EventConnection alive!
        SignalConnection connect(const std::function<void()> &f, const shared_ptr<IExecutor> &executor = DefaultExecutor::instance());
        void signal();
        Connector<EventSignal> connector();
        
        
    private:
        EventSignal(const EventSignal &rhs); // delete
        EventSignal &operator=(const EventSignal &rhs); // delete
        boost::signals2::signal<void()> _signal;
    };

    typedef Connector<EventSignal> EventConnector;
    
    
    template<class B>
    void EventSignal::connect(void(B::*method)(), const shared_ptr<B> sp, const shared_ptr<IExecutor> &executor)
    {
        weak_ptr<B> wp = sp;
        auto bound = [executor, wp, method]{
            
            auto ssp = wp.lock();
            if (ssp) {
                executor->addTask([ssp, method]{
                    (ssp.get()->*method)();
                });
            }
        };
        
        auto slotType = boost::signals2::signal<void()>::slot_type(bound);
        auto tracked = slotType.track_foreign(sp);
    
        _signal.connect(tracked);
    }
    
}

#endif /* defined(__G_Ear_iOS__Signal__) */
