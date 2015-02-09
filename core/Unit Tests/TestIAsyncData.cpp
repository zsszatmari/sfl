//
//  TestIAsyncData.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/22/13.
//
//


#include "tut/tut.hpp"
#include "Downloader.h"
#include "Thread.h"



using namespace Gear;

namespace tut
{
    using namespace Base;
    
    struct asyncinfo
    {
    };
    
    typedef test_group<asyncinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("IAsyncData");
    
    class AsyncTest final : public IAsyncData
    {
    public:
        AsyncTest() : _available(0)
        {
        }
        
        virtual int offsetAvailable(int offset) const
        {
            return _available > offset;
        }
        
        virtual void access(int offset, const function<void(const char *ptr, int available)> &f)
        {
        }
        
        virtual int totalLength() const
        {
            return 0;
        }
        
        virtual bool finished() const
        {
            return 0;
        }
        
        virtual int failed() const
        {
            return 0;
        }
        
        virtual void seek(int offset)
        {
        }
        
        void signal()
        {
            IAsyncData::signal();
        }
        
        int _available;

    };
    
    template<>
    template<>
    void testobject::test<1>()
    {
        AsyncTest test;
        
        test._available = 10;
        shared_ptr<atomic_bool> val(new atomic_bool(false));
        test.waitAsync(3, [=]{
            *val = true;
        });
        
        ensure("already", *val);
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        AsyncTest test;
        
        shared_ptr<atomic_bool> val(new atomic_bool(false));
        test.waitAsync(3, [=]{
            *val = true;
        });
        
        milliseconds w(10);
        sleep_for(w);
        ensure("notsignalled", *val == false);
        
        test._available = 10;
        test.signal();
        sleep_for(w);
        
        ensure("signalled", *val);
        
        // we should endure a second signal without deadlock!
        test.signal();
        sleep_for(w);
        ensure("signalled2", *val);
    }
    
    template<>
    template<>
    void testobject::test<3>()
    {
        AsyncTest test;
        
        test._available = 10;
        test.waitSync(3);
        
        ensure("syncalready", 1);
    }
    
    template<>
    template<>
    void testobject::test<4>()
    {
        shared_ptr<AsyncTest> pTest = shared_ptr<AsyncTest>(new AsyncTest());
        AsyncTest &test = *pTest;
        
        shared_ptr<atomic_bool> val(new atomic_bool(false));
        
        Thread t([=]{
            pTest->waitSync(3);
            *val = true;
        });
        t.detach();
        
        milliseconds w(10);
        sleep_for(w);
        
        ensure("notsignalledsync", *val == false);
        
        test._available = 10;
        test.signal();
        sleep_for(w);
        
        ensure("signalledsync", *val);
    }
}