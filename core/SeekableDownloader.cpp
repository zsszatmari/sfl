//
//  SeekableDownloader.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 09/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <sstream>
#include "SeekableDownloader.h"
#include "HttpDownloader.h"
#include "IApp.h"
#include "IPreferences.h"

namespace Gear
{
#define method SeekableDownloader::
    
    shared_ptr<ISeekDownloader> method create(const string &url, const map<string,string> &headers)
    {
        return shared_ptr<ISeekDownloader>(new SeekableDownloader(url, headers));
    }
    
    method SeekableDownloader(const string &url, const map<string,string> &headers) :
        _url(url),
        _headers(headers),
        _seekOffset(0)
    {
        _totalLength = 0;
        create(0);
    }
    
    void method create(int seekOffset)
    {
        auto headers = _headers;
        if (seekOffset > 0) {
            std::stringstream s;
            // Range: bytes=666-
            s << "bytes=";
            s << seekOffset;
            s << "-";
            headers.insert(make_pair("Range", s.str()));
        }
            
        _downloader = HttpDownloader::create(_url, headers);
        _seekOffset = seekOffset;
    }
    
    void method seek(int offset)
    {
        //std::cout << "seeking to " << offset << std::endl;
        // TODO: VPNMode is improper now because we are not storing whole songs in memory
        /*
        if (IApp::instance()->preferences().boolForKey("VPNMode")) {
            return;
        }*/
        
        // don't seek if enough is fetched already
        if (offset >= _seekOffset && offsetAvailable(offset) > 0) {
            
            //std::cout << std::hex << this << std::dec << " not seek. seeking to " << offset << "/" << _seekOffset<< std::endl;
            return;
        }
        
        //std::cout << std::hex << this << std::dec <<" do seek " << offset << std::endl;
        
        create(offset);
        _downloader->start();
    }
    
    void method waitAsync(int offset, const function<void()> &f)
    {
        _downloader->waitAsync(offset - _seekOffset, f);
    }
    
    void method waitSync(int offset)
    {
        _downloader->waitSync(offset - _seekOffset);
    }
    
    int method offsetAvailable(int offset) const
    {
        return _downloader->offsetAvailable(offset - _seekOffset);
    }
    
    void method accessChunk(int offset, const function<void(const char *ptr, int available)> &f)
    {
        _downloader->accessChunk(offset - _seekOffset, f);
    }
    
    int method totalLength() const
    {
        // after seek, the previous length is unknown, use the old value
        auto downloaderLength = _downloader->totalLength();
        if (downloaderLength != 0) {
            _totalLength = _seekOffset + downloaderLength;
        }
        return _totalLength;
    }
    
    bool method finished() const
    {
        return _downloader->finished();
    }
    
    int method failed() const
    {
        return _downloader->failed();
    }
    
    void method start()
    {
        _downloader->start();
    }
}
