//
//  stdplus.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/7/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef G_Ear_iOS_stdplus_h
#define G_Ear_iOS_stdplus_h

#include "Environment.h"

#ifdef USE_BOOST_INSTEAD_OF_CPP11
#error this is deprecated, c++11 is now mature enough
#endif

//#define USE_BOOST_LOCALES
// boost or cpp, in the common namespace
//#define USE_CPP_LOCALES


#if TARGET_OS_IPHONE
    #define FORCE_BOOST_ATOMIC
    // this is needed because iOS 6.1 does not have proper atomics in std
    // this, in turn, forces us to use boost::shared_ptr, see atomic_compare_exchange_strong for example 
#endif
#ifdef FORCE_BOOST_ATOMIC

    #define ATOMIC_NS boost
    #define ATOMIC_H "boost/atomic.hpp"
    #define MEMORY_NS boost
    #define MEMORY_H "boost/shared_ptr.hpp"
    #define WEAK_H "boost/weak_ptr.hpp"
    #define SHAREDFROMTHIS_H "boost/enable_shared_from_this.hpp"

    namespace boost
    {
        template<class T>
        class shared_ptr;

        template<class T>
        class weak_ptr;

        template<class T>
        bool atomic_compare_exchange(shared_ptr<T>* p,
                                        shared_ptr<T>* expected,
                                        shared_ptr<T> desired);

        template<class T>
        bool atomic_compare_exchange_strong(boost::shared_ptr<T>* p,
                                            boost::shared_ptr<T>* expected,
                                            boost::shared_ptr<T> desired)
        {
            return boost::atomic_compare_exchange(p, expected, desired);
        }

        template<class T>
        class enable_shared_from_this;
    }

    using boost::shared_ptr;
    using boost::weak_ptr;
    using boost::enable_shared_from_this;

    #define UNIQUE_H <boost/interprocess/smart_ptr/unique_ptr.hpp>
    namespace boost
    {
        template<typename T>
        struct Deleter {
            void operator()(T *p)
            {
                delete p;
            }
        };
    }
    #define ALLOW_UNIQUE_PTR \
        namespace boost{\
            \
            template<typename T> using unique_ptr = boost::interprocess::unique_ptr<T, Deleter<T>>;\
        }
    /*
    namespace boost
    {
        //#define BOOST_NO_EXCEPTIONS
        #include <boost/interprocess/smart_ptr/unique_ptr.hpp>

        template<typename T>
        struct Deleter {
            void operator()(T *p)
            {
                delete p;
            }
        };
        
        template<typename T>
        using unique_ptr = boost::interprocess::unique_ptr<T, Deleter<T>>;
    }*/

#else

    #define ATOMIC_NS std
    #define ATOMIC_H <atomic>
    #define MEMORY_NS std
    #define MEMORY_H <memory>
    #define WEAK_H <memory>
    #define SHAREDFROMTHIS_H <memory>
    #define UNIQUE_H <memory>
    #define ALLOW_UNIQUE_PTR

#endif

#define THREAD_NS std
#define THREAD_H <thread>
#define MUTEX_H <mutex>
#define RECURSIVE_MUTEX_H <mutex>
#define LOCK_GUARD_H <mutex>
#define CONDITION_VARIABLE_H <condition_variable>
#define FUTURE_H <future>

#define CHRONO_NS std::chrono
#define CHRONO_H <chrono>

/*
#ifdef ANDROID
namespace Base
{
    template<typename T>
    class atomic
    {
    public:
        atomic() : _value((T)0)
        {
        }

        atomic(const T value) : _value(value)
        {
        }

        atomic<T> &operator=(const T &rhs)
        {
            lock_guard<mutex> l(_valueMutex);
            _value = rhs;
            return *this;
        }

        operator T() const
        {
            lock_guard<mutex> l(_valueMutex);
            return _value;
        }

        bool compare_exchange_strong(T& expected, T desired)
        {
            lock_guard<mutex> l(_valueMutex);
            T old = _value;
            if (old == expected) {
                _value = desired;
                return true;
            }
            else {
                return false;
            }
        }

        bool compare_exchange_weak(T& expected, T desired)
        {
            return compare_exchange_weak(expected, desired);
        }

        T operator+=(T arg)
        {
            lock_guard<mutex> l(_valueMutex);
            _value = _value + arg;
            return _value;
        }

        T operator++()
        {
            // pre increment
            return operator+=(1);
        }

        T operator++(int)
        {
            // post increment
            return operator+=(1) - 1;
        }

    private:
        atomic(const atomic<T> &rhs); //delete
        atomic<T> &operator=(const atomic<T> &rhs); //delete
        T _value;
        mutable mutex _valueMutex;
    };
    typedef atomic<int> atomic_int;
}
#endif
 */

#define UNORDERED_NS std
#define UNORDERED_SET_H <unordered_set>
#define UNORDERED_MAP_H <unordered_map>
#define CHRONO_NS std::chrono
#define CHRONO_H <chrono>

/*namespace std
{
    template< class R, class... Args >
    class function;
}
using std::function;*/

namespace std
{
}
#define HASH_H <functional>
namespace HASH_NS = std;

// u16string deprecated, won't use it anymore!
#define u(x) x

#endif
