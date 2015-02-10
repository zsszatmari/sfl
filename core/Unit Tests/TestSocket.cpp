//
//  TestSocket.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/12/13.
//
//

#include "tut/tut.hpp"
#include "Socket.h"
#include "SSLSocket.h"

using namespace Gear;

namespace tut
{
    struct testsocket
    {
        //       struct keepee{ int data; };
    };
    
    typedef test_group<testsocket> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("Socket");
    
    template<>
    template<>
    void testobject::test<1>()
    {
        Socket socket("www.treasurebox.hu");
        ensure_equals("connect", socket.connect(), true);
        
        socket.write("GET /verify/1.5.txt HTTP/1.1\r\nHost:www.treasurebox.hu\r\nConnection:Close\r\n\r\n");
        vector<char> v;
        socket.read(v);
        
        string s(v.data());
        ensure_equals("notfail", socket.failed(), false);
        ensure_equals("notfinish", socket.finished(), false);
        
        ensure("gotstring", s.length() == 279);
        
        socket.read(v);
        ensure_equals("notfail2", socket.failed(), false);
        ensure_equals("finish", socket.finished(), true);
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        SSLSocket socket("www.treasurebox.hu");
        ensure_equals("connect", socket.connect(), true);
        
        socket.write("GET /verify/1.5.txt HTTP/1.1\r\nHost:www.treasurebox.hu\r\nConnection:Close\r\n\r\n");
        vector<char> v;
        socket.read(v);
        
        string s(v.data());
        ensure_equals("notfail", socket.failed(), false);
        ensure_equals("notfinish", socket.finished(), false);
        
        ensure("gotstring", s.length() == 279);
        
        socket.read(v);
        ensure_equals("notfail2", socket.failed(), false);
        ensure_equals("finish", socket.finished(), true);
    }
}
