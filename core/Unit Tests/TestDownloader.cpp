//
//  TestDownloader.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/18/13.
//
//


#include "tut/tut.hpp"
#include "Downloader.h"


using namespace Gear;

namespace tut
{
    struct downloaderinfo
    {
    };
    
    typedef test_group<downloaderinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("Downloader");
    
    class MockService
    {
    };
    
    class MockSocket
    {
    public:
        static void reset()
        {
        }
        
        static void notify()
        {
        }
    };
    
    //typedef DownloaderTemplate<MockService, MockSocket> DownloaderTest;
    
    template<>
    template<>
    void testobject::test<1>()
    {
        // not implemented yet, but will be necessary for other platforms
        //ensure("appleonlyyet", 0);
        
        /*MockSocket::reset();
        
        DownloaderTest download("www.google.com", "header");
        
        ensure("noavailable", 0 == download.offsetAvailable(300));
        bool notified = false;
        int availInNotify = 0;
        download.waitForOffset(300, [&]{
            notified = true;
            
            ensure("available1", 800 == download.offsetAvailable(200));
            ensure("available2", 700 == download.offsetAvailable(300));
            
            download.access(300, [&](char *buf, int available){
                availInNotify = available;
            });
        });
        ensure("notnotified", !notified);
        
        MockSocket::notify();
        ensure("notified", notified);
        
        ensure("availInNotify", availInNotify == 700);*/
    }
}