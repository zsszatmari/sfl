//
//  TestSerialExecutor.cpp
//  Base
//
//  Created by Zsolt Szatmári on 5/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "tut/tut.hpp"
#include "SerialExecutor.h"
#include "stdplus.h"

using namespace Base;

namespace tut
{
    struct testinfo
    {
 //       struct keepee{ int data; };
    };
  
    typedef test_group<testinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("SerialExecutor");
    
    template<>
    template<>
    void testobject::test<1>()
    {
        int value = 0;
        
        {
            SerialExecutor executor;
            
            executor.addTask([&](){
                value = 1;
            });
            
            //executor.addTask(task);
            
        }
        ensure("basic", value == 1);
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        int value = 0;
        SerialExecutor executor;
        
        executor.addTaskAndWait([&] {
            value = 1;
        });
        
        ensure("wait", value == 1);
    }
    
    template<>
    template<>
    void testobject::test<3>()
    {
        int value1 = 0;
        int value2 = 0;
        SerialExecutor executor;
        
        executor.addTask([&] {
            value1 = 1;
        });
        
        executor.addTaskAndWait([&] {
            value2 = 1;
        });
        
        ensure("combined1", value1 == 1);
        ensure("combined2", value2 == 1);
    }
   
    class test_exception : public std::exception
    {
        virtual const char* what()
        {
            return "hihi";
        }
    };
    
    template<>
    template<>
    void testobject::test<4>()
    {
        SerialExecutor executor;
        
        bool thrown = false;
        try {
            executor.addTaskAndWait([] {
                throw test_exception();
            });
        } catch(test_exception &) {
            thrown = true;
#ifdef USE_BOOST_INSTEAD_OF_CPP11
            // this might be good enough for boost, but...
            
        } catch(std::exception &e) {
            //cout << e.what() << endl;
            //ensure("notgood", 0);
            thrown = true;
#endif
        }
        
        ensure("exception", thrown);
    }
    
    template<>
    template<>
    void testobject::test<5>()
    {
        SerialExecutor executor;
        
        bool success = false;
        executor.addTaskAndWait([&] {
            
            int value = 0;
            executor.addTaskAndWait([&] {
                value = 1;
            });
            if (value == 1) {
                success = true;
            }
        });
    
        ensure("localthread", success);
    }
};
