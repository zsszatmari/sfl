//
//  SeekableDownloader.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 09/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__SeekableDownloader__
#define __G_Ear_core__SeekableDownloader__

#include "stdplus.h"
#include "ISeekDownloader.h"
#include MEMORY_H

namespace Gear
{
    using std::function;

    class SeekableDownloader : public ISeekDownloader
    {
    public:
        static shared_ptr<ISeekDownloader> create(const string &url, const map<string,string> &headers);
    
        virtual void seek(int offset);
        
        virtual void waitAsync(int offset, const function<void()> &f);
        void waitSync(int offset);
        virtual int offsetAvailable(int offset) const;
        
        virtual void accessChunk(int offset, const function<void(const char *ptr, int available)> &f);
        
        virtual int totalLength() const;
        virtual bool finished() const;
        virtual int failed() const;
        virtual void start();

        
    private:
        SeekableDownloader(const string &url, const map<string,string> &headers);
        
        const string _url;
        const map<string,string> _headers;
        
        void create(int seekOffset);
        
        shared_ptr<IDownloader> _downloader;
        int _seekOffset;
    };
}

#endif /* defined(__G_Ear_core__SeekableDownloader__) */
