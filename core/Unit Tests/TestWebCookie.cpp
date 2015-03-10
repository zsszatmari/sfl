//
//  TestWebCookie.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/6/13.
//
//

#include "tut/tut.hpp"
#include "WebCookie.h"

using namespace Gear;

namespace tut
{
    struct testwebcookie
    {
        //       struct keepee{ int data; };
    };
    
    typedef test_group<testwebcookie> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("WebCookie");
    
    template<>
    template<>
    void testobject::test<1>()
    {
        // google's servers don't get the standard right...
        string str("SID=DQAAANIAAAChueQU-uUTcj6r-DCXawEJdTr7JzfTH4tJsSxMOuo27CE1Rel1KJvEhka7OPJQkfjL4PizWqmrmSgn-apIo6YXLUQrrABIw_LFSAXdLP6m94PP4wJD-RrZNxsIp8FVOq2SEoGazloN3ikGjKncb7pgPW48nfh3yzVf2IMQN1mnmGC0veO6JZxTXMjdHpIhmFDeOgDFwPaGj8Sdbz5W-ZM_U8pU5z8brSO0esSsSLY8tXCKvNsW_GaL-b6c5Qsa5tyL_MDJ7n417lCcUMeJByW0F84V_aSLqtwQQV_UzKlbcg;Domain=.google.com;Path=/;Expires=Sun, 06-Sep-2015 10:20:42 GMT, xt=CjUKATASMEFNLVdiWGhwRmFYOThYRFpLTTFpY0I1N2lPc1llR0xvUlE6MTM3ODQ2Mjg0MjI0NA==; Secure");
        
        auto cookies = WebCookie::cookies(str, "");
        ensure_equals("count", cookies.size(), 2);
        ensure("value", cookies.at(1).value() == "CjUKATASMEFNLVdiWGhwRmFYOThYRFpLTTFpY0I1N2lPc1llR0xvUlE6MTM3ODQ2Mjg0MjI0NA==");
    }
}