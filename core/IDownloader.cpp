//
//  Downloader.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/18/13.
//
//

#include <iostream>
#include "IDownloader.h"
#include "HttpDownloader.h"
#include MEMORY_H

namespace Gear
{
#define method IDownloader::
    
    method IDownloader()
    {
        _totalLength = 0;
    }
    
    method ~IDownloader()
    {
    }
    
    void method setTotalLength(int total, bool forced)
    {
#ifdef DEBUG
        //std::cout << "got total length: " << total << std::endl;
#endif
        if (forced) {
            _totalLength = total;
        } else {
            int expected = 0;
            _totalLength.compare_exchange_strong(expected, total);
        }
    }
    
    int method totalLength() const
    {
        return _totalLength;
    }
    
    template<class T>
    static T asType(IDownloader &downloader)
    {
        size_t avail = downloader.offsetAvailable(0);
        size_t offset = 0;
        
        T ret;
        ret.reserve(avail);
        while (offset < avail) {
            downloader.accessChunk(offset, [&](const char *ptr, int available){
                ret.insert(ret.end(), ptr, ptr+available);
                offset += available;
            });
        }
        //std::cout << "got data: " << avail << "/" << ret.size() << std::endl;
        return ret;

    }
    
    method operator string()
    {
        //std::cout << "data itself: " << asType<string>(*this) << "\n";
        return asType<string>(*this);
    }
    
    method
    operator vector<char>()
    {
        return asType<vector<char>>(*this);
    }
    
    void method waitUntilFinished()
    {
        int offset = 0;
        while (!finished()) {
            waitSync(offset);
            offset = offsetAvailable(0) + 1;
        }
    }

#undef method
    
    IDownloaderFactory::~IDownloaderFactory()
    {
    }
    
    shared_ptr<IDownloader> DownloaderFactory::create(const string &url, const map<string,string> &headers) const
    {
        return HttpDownloader::create(url, headers);
    }
}