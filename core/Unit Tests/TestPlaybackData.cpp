//
//  TestPlaybackData.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/17/13.
//
//

#include "stdplus.h"
#include "tut/tut.hpp"
#include "PlaybackData.h"
#include "GearUtility.h"
#include "Thread.h"


using namespace Gear;

namespace tut
{
    using namespace Base;
    
    struct playbackinfo
    {
    };
    
    typedef test_group<playbackinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("PlaybackData");
    
    class MockDownloader final : public IDownloader
    {
        mutex m;
        condition_variable wakeup;
        bool notified;
        
    public:
        MockDownloader(const string &url, const map<string,string> &headers) : _offsetAvailable(0), _finished(false)
        {
        }
        
        virtual ~MockDownloader()
        {
        }
        
        virtual int offsetAvailable(int offset) const
        {
            if (_offsetAvailable > offset) {
                return _offsetAvailable - offset;
            } else {
                return 0;
            }
        }

        virtual void access(int offset, const function<void(const char *ptr, int available)> &f)
        {
            waitSync(offset);
            f(NULL, 0);
        }
        
        virtual bool finished() const
        {
            return _finished;
        }
        
        virtual int failed() const
        {
            return false;
        }
        
        virtual void seek(int offset)
        {
        }
        
        void signal()
        {
            IAsyncData::signal();
        }
        
        void start()
        {
        }
        
        int _offsetAvailable;
        bool _finished;
    };
    
    class MockDownloaderFactory : public IDownloaderFactory
    {
    public:
        virtual shared_ptr<IDownloader> create(const string &url, const map<string,string> &headers) const
        {
            MockDownloader *ret = new MockDownloader(url, headers);
            downloaders.push_back(ret);
            
            return shared_ptr<IDownloader>(ret);
        }
        
        // we violate unique_ptr for the sake of testing!
        mutable vector<MockDownloader *> downloaders;
    };
    
    template<>
    template<>
    void testobject::test<1>()
    {
        
        string u[] = {"http://a/1.dat", "http://a/2.dat"};
        MockDownloaderFactory factory;
        shared_ptr<PlaybackData> pData(new PlaybackData(init<vector<string>>(u), factory));
        auto &data = *pData;
        
        ensure("factorycount", factory.downloaders.size() == 2);
        ensure("notavailable", !data.finished());
        ensure("noavailable", 0 == data.offsetAvailable(0));
        
        shared_ptr<atomic_bool> pVal1(new atomic_bool(false));
        auto &val1 = *pVal1;
        shared_ptr<atomic_bool> pVal2(new atomic_bool(false));
        auto &val2 = *pVal2;
        //shared_ptr<atomic_bool> pVal3(new atomic_bool(false));
        //auto &val3 = *pVal3;
        
        //boost:packaged_task<void> task([=]{
        //});
        
        Thread t([=]{
            pData->access(0, [=](const char *, int){
                *pVal1 = true;
            });
            
            pData->access(1200, [=](const char *, int){
                *pVal2 = true;
            });
        });
        
        t.detach();
        
        milliseconds w(40);
        sleep_for(w);
        
        ensure("w", val1 == false);
        ensure("offset1", data.offsetAvailable(5) == 0);
        
        factory.downloaders[0]->_offsetAvailable = 1000;
        factory.downloaders[0]->setTotalLength(1000);
        //factory.downloaders[0]->_finished = true;
        factory.downloaders[0]->signal();
        
        sleep_for(w);
        ensure("offset2", data.offsetAvailable(5) == 995);
        ensure("w2", val1 == true);
        
        ensure("w3", val2 == false);
        factory.downloaders[1]->_offsetAvailable = 2000;
        factory.downloaders[1]->signal();
        sleep_for(w);
        int available = data.offsetAvailable(5);
        // still 995 because we haven't finished yet!
        ensure("offset3", available == 995);
        factory.downloaders[0]->_finished = true;
        factory.downloaders[0]->setTotalLength(1000);
        factory.downloaders[0]->signal();
        available = data.offsetAvailable(5);
        // continously available data, so it is not 2995!
        ensure("offset4", available == 995);
        
        ensure("total1", data.totalLength() == 0);
        factory.downloaders[1]->setTotalLength(2000);
        ensure("total2", data.totalLength() == 3000);
        
        // let the second access be called!
        sleep_for(w);
        ensure("w4", val2 == true);
        
        // throw exceptions if necessary
        //task.get_future().get()
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        string u[] = {"http://a/1.dat", "http://a/2.dat"};
        MockDownloaderFactory factory;
        shared_ptr<PlaybackData> pData(new PlaybackData(init<vector<string>>(u), factory));
        auto &data = *pData;
        
        factory.downloaders[0]->_offsetAvailable = 1000;
        factory.downloaders[0]->setTotalLength(2000);
        shared_ptr<atomic_bool> pVal1(new atomic_bool(false));
        auto &val1 = *pVal1;
        
        int avail = data.offsetAvailable(1000);
        ensure("offset", avail == 0);
        
        Thread t([=]{
            pData->access(1000, [=](const char *ptr, int available){
                
                // this either
                *pVal1 = true;
            });
        });
        t.detach();
        
        factory.downloaders[0]->signal();
        milliseconds w(10);
        sleep_for(w);
        ensure("borderr", val1 == false);
    }

    
    template<>
    template<>
    void testobject::test<3>()
    {
        string u[] = {"http://a/1.dat", "http://a/2.dat"};
        MockDownloaderFactory factory;
        shared_ptr<PlaybackData> pData(new PlaybackData(init<vector<string>>(u), factory));
        auto &data = *pData;
        
        factory.downloaders[0]->_offsetAvailable = 1000;
        factory.downloaders[0]->setTotalLength(2000);
        shared_ptr<atomic_bool> pVal1(new atomic_bool(false));
        auto &val1 = *pVal1;
        shared_ptr<atomic_bool> pVal2(new atomic_bool(false));
        auto &val2 = *pVal2;
        
        data.waitAsync(1000,[=]{
            //mustn't be called yet!
            *pVal1 = true;
        });
        
        milliseconds w(10);
        sleep_for(w);
        
        ensure("border", val1 == false);

        Thread t([=]{
            pData->access(1000, [=](const char *ptr, int available){
                
                // this either
                *pVal2 = true;
            });
        });
        t.detach();
        
        factory.downloaders[0]->signal();
        sleep_for(w);
        ensure("border2", val2 == false);
        
        
        factory.downloaders[0]->_offsetAvailable = 1200;
        factory.downloaders[0]->signal();
        sleep_for(w);
        
        ensure("border1after", val1 == true);
        ensure("border2after", val2 == true);
    }
    
    template<>
    template<>
    void testobject::test<4>()
    {
        string u[] = {"http://r5---sn-po8puxa-c0qe.c.doc-0-0-sj.sj.googleusercontent.com/videoplayback?id=820a5652b00f2676&itag=25&source=skyjam&o=16406516421120510485&range=0-39706&segment=0&ip=0.0.0.0&ipbits=0&expire=1369297651&sparams=id,itag,source,o,range,segment,ip,ipbits,expire&signature=4030F7FADCF72349B95807D791882D1531A68F9E.AA0BFDFB4517E897C6212A85B94CF7F8D89CFC5C&key=sj3&ms=au&newshard=yes&mv=m&mt=1369297578",
                      "http://r5---sn-po8puxa-c0qe.c.doc-0-0-sj.sj.googleusercontent.com/videoplayback?id=820a5652b00f2676&itag=25&source=skyjam&o=16406516421120510485&range=37615-119118&segment=1&ip=0.0.0.0&ipbits=0&expire=1369297652&sparams=id,itag,source,o,range,segment,ip,ipbits,expire&signature=2E82C520478626C63AF1687051BD24ADA7AE0EB2.82CD6C7097518EFF3A298B2EA507C1989C8FE0AC&key=sj3&ms=au&newshard=yes&mv=m&mt=1369297578",
                      "http://r5---sn-po8puxa-c0qe.c.doc-0-0-sj.sj.googleusercontent.com/videoplayback?id=820a5652b00f2676&itag=25&source=skyjam&o=16406516421120510485&range=117027-278987&segment=2&ip=0.0.0.0&ipbits=0&expire=1369297654&sparams=id,itag,source,o,range,segment,ip,ipbits,expire&signature=270156E76A7E1BD2F0CCAE8DBF9BB254073E3B2F.72B0874A44BE3B5CDA1007C386FBE031C272D64A&key=sj3&ms=au&newshard=yes&mv=m&mt=1369297578"};
        
        MockDownloaderFactory factory;
        shared_ptr<PlaybackData> pData(new PlaybackData(init<vector<string>>(u), factory));
        auto &data = *pData;
        
        //cout << "length0: " << factory[down]
        
        ensure("totallength", data.totalLength() == 278987);
    }
}