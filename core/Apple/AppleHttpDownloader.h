//
//  HttpDownloader.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/20/13.
//
//

#ifndef __G_Ear__HttpDownloader__
#define __G_Ear__HttpDownloader__

#include <dispatch/dispatch.h>
#include <map>
#include <vector>
#include "stdplus.h"
#include "IDownloader.h"
#include "SerialExecutor.h"
#include CHRONO_H
#include SHAREDFROMTHIS_H

namespace Gear
{
    using std::function;

    class AppleHttpDownloader : public IDownloader, public MEMORY_NS::enable_shared_from_this<AppleHttpDownloader>
    {
    public:
        static shared_ptr<AppleHttpDownloader> create(const string &url);
        static shared_ptr<AppleHttpDownloader> create(const string &url, const map<string,string> &extraHeaders, const string &method = "GET", const string &body = "");
        static bool isOnline();
        
        
        virtual ~AppleHttpDownloader();
        //HttpDownloader(const string &url, const IDownloaderFactory & downloaderFactory = DownloaderFactory());
        
        virtual void start();
        virtual int offsetAvailable(int offset) const;
        virtual void accessChunk(int offset, const function<void(const char *ptr, int available)> &f);
        
        virtual bool finished() const;
        virtual int failed() const;
        
        virtual void waitAsync(int offset, const function<void()> &f);
        
        
        // these are not part of the public interface but temporarily public because of objective-c++ hackery!
        
        void didReceiveData(void *delegate, void *objcData);
        void didFail(void *delegate, int code = 1);
        void didFinish(void *delegate);
        
    private:
        AppleHttpDownloader(const string &url);
        //AppleHttpDownloader(const string &url, int rangeStart);
        AppleHttpDownloader(const string &url, const map<string,string> &extraHeaders, const string &method = "GET", const string &body = "");
        
        void *connection;
        void *connectionDelegate;
        
        void init(const string &url, const map<string,string> &extraHeaders);
        void startDownload(const string &url, const map<string,string> &extraHeaders);
        void prepareDownload(const string &url, const map<string,string> &extraHeaders);
        void signal();
        
        bool _started;
        CHRONO_NS::steady_clock::time_point _startTime;
        
        int _tryCount;
        const string _url;
        const string _method;
        const string _body;
        vector<char> data;
        const map<string,string> _extraHeaders;
        Base::SerialExecutor _queue;
        dispatch_semaphore_t semaphore;
        ATOMIC_NS::atomic_int _available;
        ATOMIC_NS::atomic_int _startPosition;
        ATOMIC_NS::atomic<bool> _finished;
        class Mutable
        {
        public:
            //Mutable();
            ATOMIC_NS::atomic<int> _failed;
        };
        mutable Mutable m;
        
        shared_ptr<ATOMIC_NS::atomic<bool>> _destructing;
        //int _rangeStart;
        
        //const unique_ptr<IDownloader> downloader;
    };
}

#endif /* defined(__G_Ear__HttpDownloader__) */
