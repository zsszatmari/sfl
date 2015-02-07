//
//  HttpDownloader.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/20/13.
//
//

#ifndef __G_Ear__GenericHttpDownloader__
#define __G_Ear__GenericHttpDownloader__

#include <map>
#include <vector>
#include "stdplus.h"
#include "SerialExecutor.h"
#include "IDownloader.h"
#include "Chain.h"
#include "AtomicPtr.h"
#include CHRONO_H
#include SHAREDFROMTHIS_H
using MEMORY_NS::weak_ptr;

// missing SSL Cert verification!!!
// missing support for proxy (http, https, socks4, socks5)

namespace Gear
{
    using std::function;

    class GenericHttpDownloader : public IDownloader, public MEMORY_NS::enable_shared_from_this<GenericHttpDownloader>
    {
    public:
        static shared_ptr<GenericHttpDownloader> create(const string &url);
        static shared_ptr<GenericHttpDownloader> create(const string &url, const map<string,string> &extraHeaders, const string &method = "GET", const string &body = "");
        virtual ~GenericHttpDownloader();
        //HttpDownloader(const string &url, const IDownloaderFactory & downloaderFactory = DownloaderFactory());
        
        virtual void start();
        virtual int offsetAvailable(int offset) const;
        virtual void accessChunk(int offset, const function<void(const char *ptr, int available)> &f);
        
        virtual bool finished() const;
        virtual int failed() const;
        
        virtual void waitAsync(int offset, const function<void()> &f);
        
    private:
        GenericHttpDownloader(const string &url);
        GenericHttpDownloader(const string &url, int rangeStart);
        GenericHttpDownloader(const string &url, const map<string,string> &extraHeaders, const string &method = "GET", const string &body = "");
        
        void *connection;
        void *connectionDelegate;
        
        void init(const string &url, const map<string,string> &extraHeaders);
        void startDownload(const string &url, const map<string,string> &extraHeaders);
        void prepareDownload(const string &url, const map<string,string> &extraHeaders);
        void signal();
        static string doDownload(const weak_ptr<GenericHttpDownloader> &wThis, const string &url);
        static void download(const weak_ptr<GenericHttpDownloader> &wThis, const string &url);
        
        ATOMIC_NS::atomic<bool> _started;
        CHRONO_NS::steady_clock::time_point _startTime;
        Base::SerialExecutor queue;
        
        const string _url;
        const string _method;
        const string _body;
        int _validConnection;
        map<string, string> _headers;
        AtomicPtr<Chain<char>> _payload;
        // this means not only seek start position, but positioning due to that we only keep a window in memory
        ATOMIC_NS::atomic_int _startPosition;
        ATOMIC_NS::atomic<bool> _finished;
        class Mutable
        {
        public:
            //Mutable();
            ATOMIC_NS::atomic<int> _failed;
        };
        mutable Mutable m;
        
        THREAD_NS::condition_variable _shallContinue;
        THREAD_NS::mutex _shallContinueMutex;
        
        //int _rangeStart;
        
        //const unique_ptr<IDownloader> downloader;
    };
}

#endif /* defined(__G_Ear__HttpDownloader__) */
