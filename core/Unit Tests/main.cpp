//
//  main.cpp
//  Unit Tests
//
//  Created by Zsolt Szatmári on 5/7/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <iostream>
#include "tut/tut.hpp"
#include "GearUtility.h"
#include <memory>
#include "stdplus.h"


using std::cout;
using std::endl;

namespace tut
{
    test_runner_singleton runner;
}

class callback : public tut::callback
{
public:
	callback() : issues(0)
	{
	}
    
    int issues;
    
    void run_started() {}
    
    void test_completed(const tut::test_result &tr)
    {
        std::string result;
        
        switch(tr.result) {
            case tut::test_result::result_type::ok:
                result = "ok";
                break;
            case tut::test_result::result_type::fail:
                result = "fail";
                break;
            case tut::test_result::result_type::ex:
                result = "ex";
                break;
            case tut::test_result::result_type::warn:
                result = "warn";
                break;
            case tut::test_result::result_type::term:
                result = "term";
                break;
            case tut::test_result::result_type::ex_ctor:
                result = "ex_ctor";
                break;
            case tut::test_result::result_type::rethrown:
                result = "rehrown";
                break;
            case tut::test_result::result_type::dummy:
                result = "dummy";
                break;
        }
        std::string extra;
        if (tr.result != tut::test_result::result_type::ok) {
            issues++;
            extra = tr.message;
        }
        
        std::cout << tr.group << " " << tr.test << ": " << result;
        if (extra.length()) {
            cout << " (" << extra << ")";
        }
        
        cout << endl;
    }
    
    void run_completed( ){}
};


#if TARGET_OS_IPHONE
#include "fakemain.h"
#endif

int main(int argc, const char * argv[])
{
    auto runTests = []{
        Base::localeInit();
        
        callback clbk;
        tut::runner.get().set_callback(&clbk);
        //tut::runner.get().run_tests();
        
        tut::test_result r;
        //tut::runner.get().run_test("GooglePlayConceiver", 11, r);
        tut::runner.get().run_tests("YouTubeProtocol");
        
        //tut::runner.get().run_test("StoredSongArray", 1, r);
        
        cout << endl << "total issues: " << clbk.issues << endl;
    };
    
#if TARGET_OS_IPHONE
    std::thread t(runTests);
    fakemain(argc, argv);
#else 
    runTests();
    
	// needed because of vstudio
	getchar();
#endif
        
    
    return 0;
}

