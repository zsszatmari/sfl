//
//  TestDownloader.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/18/13.
//
//

#include <stdio.h>
#include <string.h>
#include "tut/tut.hpp"
#include "HttpDownloader.h"


using namespace Gear;

namespace tut
{
    struct httpdownloaderinfo
    {
    };
    
    typedef test_group<httpdownloaderinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("HttpDownloader");
    
    
    // functional tests..
    
    
    template<>
    template<>
    void testobject::test<1>()
    {
        char expected[] = "fdsfasfewt2332yhthgeasaerewrw211222\n";
        
        shared_ptr<IDownloader> pDownloader(HttpDownloader::create("http://www.treasurebox.hu/verify/1.4.21.txt"));
        IDownloader &downloader = *pDownloader;
        int finished = downloader.finished();
        ensure("notavailable", !finished);
        
        ensure("noavailable", 0 == downloader.offsetAvailable(0));
        shared_ptr<atomic_bool> notified(new atomic_bool(false));
        shared_ptr<atomic_bool> match(new atomic_bool(false));
        while (!downloader.finished()) {
            downloader.waitSync(downloader.offsetAvailable(0));
            
            *notified = true;
            
            if (pDownloader->finished()) {
                
                pDownloader->access(0, [&](const char *buf, int available){
                    
                    std::cout << "bytes: " << available << "(" << string(buf,available) << std::endl;
                    if (available == sizeof(expected) -1) {  // -1: terminating '\0'
                        *match = (strncmp(expected, buf, sizeof(expected)-1) == 0);
                    }
                    
                    std::cout << "bytes: " << available << "(" << string(buf,available) << ") match: " << *match << std::endl;
                });
            }
        }
        std::cout << "match: " << *match << std::endl;
        
        ensure("notified", *notified);
        ensure("notfail", !downloader.failed());
        ensure("match", *match);
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        const char expected[] = "sfasfewt2332yhthgeasaerewrw211222\n";
        
        // TODO: range requests are not yet tested!!!
        shared_ptr<IDownloader> pDownloader(HttpDownloader::create("http://www.treasurebox.hu/verify/1.4.21.txt"));
        IDownloader &downloader = *pDownloader;
        int finished = downloader.finished();
        ensure("notavailable", !finished);
        
        ensure("noavailable", 0 == downloader.offsetAvailable(2));
        shared_ptr<atomic_bool> notified(new atomic_bool(false));
        shared_ptr<atomic_bool> match(new atomic_bool(false));
        while (!downloader.finished()) {
            downloader.waitSync(downloader.offsetAvailable(0));
            *notified = true;
            
            if (pDownloader->finished()) {
                
                pDownloader->access(2, [=](const char *buf, int available){
                    if (available == sizeof(expected) -1) {  // -1: terminating '\0'
                        *match = (strncmp(expected, buf, sizeof(expected)-1) == 0);
                    }
                });
            }
        }
        
        ensure("notified", *notified);
        ensure("notfail", !downloader.failed());
        ensure("match", *match);
    }
}