//
//  TestStringUtility.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/26/13.
//
//

#include <stdio.h>
#include <string.h>
#include "tut/tut.hpp"
#include "StringUtility.h"


using namespace Gear;

namespace tut
{
    struct stringinfo
    {
    };
    
    typedef test_group<stringinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("StringUtility");
    
    
    template<>
    template<>
    void testobject::test<1>()
    {
        NSString *plain = @"sima";
        ensure("plain", convertString(plain) == string("sima"));
        
        NSString *extra = @"árvíztűrő";
        ensure("extra", convertString(extra) == string("árvíztűrő"));
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        NSString *plain = @"sima";
        ensure("plain", convertString(plain) == u("sima"));
        
        NSString *extra = @"árvíztűrő";
        ensure("extra", convertString(extra) == u("árvíztűrő"));
    }
    
    template<>
    template<>
    void testobject::test<3>()
    {
        string plain("sima");
        ensure("plain", [convertString(plain) isEqual:@"sima"]);
        
        string extra("árvíztűrő");
        ensure("extra", [convertString(extra) isEqual:@"árvíztűrő"]);
    }
    
    template<>
    template<>
    void testobject::test<4>()
    {
        string plain(u("sima"));
        ensure("plain", [convertString(plain) isEqual:@"sima"]);
        
        string extra(u("árvíztűrő"));
        ensure("extra", [convertString(extra) isEqual:@"árvíztűrő"]);
    }
    
    template<>
    template<>
    void testobject::test<5>()
    {
        NSString *plain = @"sima";
        ensure("plain", [convertString(convertString(plain)) isEqual:plain]);
                         
        NSString *extra = @"árvíztűrő";
        ensure("extra", [convertString(convertString(extra)) isEqual:extra]);
    }
}