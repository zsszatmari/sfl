//
//  TestGooglePlaySignature.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/26/13.
//
//

#include "tut/tut.hpp"
#include "GooglePlaySignature.h"


using namespace Gear;

namespace tut
{
    struct testsignature
    {
        //       struct keepee{ int data; };
    };
    
    typedef test_group<testsignature> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("GooglePlaySignature");
    
    template<>
    template<>
    void testobject::test<1>()
    {
        GooglePlaySignature s;
        s.setSalt("fe3elrdvpe2k");
        
        const string sig = s.generateForSongId("4791ca77-c1a1-3de2-9623-4ebb17ce2514");
        ensure("signature", sig == "6cfR2pXwmsm635B0l7eFMbzWLmg.");
    }
}