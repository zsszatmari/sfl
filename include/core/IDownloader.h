//
//  Downloader.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/18/13.
//
//

#ifndef __G_Ear__Downloader__
#define __G_Ear__Downloader__

#include <string>
#include <queue>
#include "stdplus.h"
#include ATOMIC_H
#include "IAsyncData.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    using std::map;
    using std::string;
    using namespace Base;
    
    class IDownloader : public IAsyncData
    {
    public:
        IDownloader();
        virtual ~IDownloader();
        
        virtual void start() = 0;
        virtual int totalLength() const;
        void setTotalLength(int total, bool forced = false);
        operator string();
        operator vector<char>();
        void waitUntilFinished();
        
    protected:
        mutable ATOMIC_NS::atomic_int _totalLength;
        
    private:
        IDownloader(const IDownloader &rhs); // delete
        IDownloader &operator=(const IDownloader &rhs); // delete
    };
    
    class IDownloaderFactory
    {
    public:
        virtual ~IDownloaderFactory();
        virtual shared_ptr<IDownloader> create(const string &url, const map<string,string> &headers) const = 0;
    };

    class DownloaderFactory : public IDownloaderFactory
    {
    public:
        virtual shared_ptr<IDownloader> create(const string &url, const map<string,string> &headers) const;
    };
    
    /*template<class Service, class Socket>
    class DownloaderTemplate final : public IDownloader
    {
    public:
        DownloaderTemplate(const string &url, const string &headers);
        virtual ~DownloaderTemplate();
        virtual int offsetAvailable(int offset);
        virtual void waitForOffset(int offset, const function<void()> &f);
        virtual void access(int offset, const function<void(char *ptr, int available)> &f);
        
    private:
        DownloaderTemplate(const DownloaderTemplate<Service,Socket> &rhs); //delete
        DownloaderTemplate<Service, Socket> operator=(const DownloaderTemplate<Service,Socket> &rhs); //delete
        vector<char> data;
        vector<pair<int, function<void()>>> waiting;
        int currentDownloadOffset;
    };
    
    typedef DownloaderTemplate<io_service, ip::tcp::socket> Downloader;

#define templatedef template<class Service, class Socket>
#define method DownloaderTemplate<Service, Socket>::
    
    templatedef
    method DownloaderTemplate(const string &url, const string &headers)
    {
    }
    
    templatedef
    method ~DownloaderTemplate()
    {
    }
    
    templatedef
    int method offsetAvailable(int offset)
    {
        
    }
    
    templatedef
    void method waitForOffset(int offset, const function<void()> &f)
    {
        
    }
    
    templatedef
    void method access(int offset, const function<void(char *ptr, int available)> &f)
    {
    }

    
#undef method
#undef templatedef*/
}

#endif /* defined(__G_Ear__Downloader__) */
