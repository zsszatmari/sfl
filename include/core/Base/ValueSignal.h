//
//  ValueSignal.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/10/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ValueSignal__
#define __G_Ear_core__ValueSignal__

#include "stdplus.h"
#include "Environment.h"

#ifdef USE_BOOST_INSTEAD_OF_CPP11
#define BOOST_NO_CXX11_VARIADIC_TEMPLATES
#define BOOST_NO_CXX11_RVALUE_REFERENCES
#endif
#include "boost/signals2.hpp"

#include "Connector.h"
#include "ValueConnection.h"
#include "DefaultExecutor.h"
#include RECURSIVE_MUTEX_H
#include LOCK_GUARD_H

#include MEMORY_H
using MEMORY_NS::shared_ptr; 
using MEMORY_NS::weak_ptr;

namespace Base
{
    template<typename ValueType>
    class ValueSignal;
    
    template<class T>
    class ValueConnector final : public Connector<ValueSignal<T>>
    {
    public:
        T value()
        {
            T ret;
            this->connect([&ret](const T &current){
                ret = current;
            });
            return ret;
        };
        
    private:
        ValueConnector(ValueSignal<T> &signal) :
            Connector<ValueSignal<T>>(signal)
        {
        }
        friend class ValueSignal<T>;
    };
    
    
    template<typename ValueType>
    class ValueSignal final
    {
    public:
        ValueSignal();
        template<class B>
        void connect(void(B::*method)(const ValueType value), const shared_ptr<B> sp, const shared_ptr<IExecutor> &executor = DefaultExecutor::instance());
        SignalConnection connect(const std::function<void(const ValueType value)> &f, const shared_ptr<IExecutor> &executor = DefaultExecutor::instance());
        void signal(const ValueType value);
        ValueConnector<ValueType> connector();
        
        ValueType lastValue() const;
        
    private:
        ValueSignal(const ValueSignal &rhs); // delete
        ValueSignal &operator=(const ValueSignal &rhs); // delete
        boost::signals2::signal<void(const ValueType)> _signal;
        
        ValueType _lastValue;
    };
    
    
    
    //template<typename T>
    //using ValueConnector = Connector<ValueSignal<T>>;
    
    
    
    template<typename ValueType>
    ValueSignal<ValueType>::ValueSignal()
    {
    }
    
    
	core_export extern THREAD_NS::recursive_mutex globalSignalMutex;
    
    template<typename ValueType>
    inline void globallock_set(ValueType &value, const ValueType rhs)
    {
        THREAD_NS::lock_guard<THREAD_NS::recursive_mutex> l(globalSignalMutex);
        value = rhs;
    }
    
    template<typename ValueType>
    inline ValueType globallock_get(const ValueType &value)
    {
        THREAD_NS::lock_guard<THREAD_NS::recursive_mutex> l(globalSignalMutex);
        return value;
    }
    
    template<typename ValueType>
    template<class B>
    void ValueSignal<ValueType>::connect(void(B::*method)(const ValueType value), const shared_ptr<B> sp, const shared_ptr<IExecutor> &executor)
    {
        typedef boost::signals2::signal<void(const ValueType)> signaltype;
     
        weak_ptr<B> wp = sp;
        auto bound = [executor, wp, method](const ValueType value){
            
            auto ssp = wp.lock();
            if (ssp) {
                executor->addTask([ssp, method, value]{
                    (ssp.get()->*method)(value);
                });
            }
        };
        
        
        typename signaltype::slot_type slot(bound);
        auto tracked = slot.track_foreign(sp);
        
        (sp.get()->*method)(globallock_get(_lastValue));
        _signal.connect(tracked);
    }
    
    template<typename ValueType>
    SignalConnection ValueSignal<ValueType>::connect(const std::function<void(const ValueType value)> &f, const shared_ptr<IExecutor> &executor)
    {
        shared_ptr<ValueConnectionToken<ValueType>> connection(new ValueConnectionToken<ValueType>(f));
        connect(&ValueConnectionToken<ValueType>::signal, connection, executor);
        return connection;
    }

    
    template<typename ValueType>
    void ValueSignal<ValueType>::signal(const ValueType value)
    {
        globallock_set(_lastValue, value);
        
        _signal(value);
    }
    
    template<typename ValueType>
    ValueConnector<ValueType> ValueSignal<ValueType>::connector()
    {
        return ValueConnector<ValueType>(*this);
    }
    
    template<typename ValueType>
    ValueType ValueSignal<ValueType>::lastValue() const
    {
        return globallock_get<ValueType>(_lastValue);
    }
}

#endif /* defined(__G_Ear_core__ValueSignal__) */
