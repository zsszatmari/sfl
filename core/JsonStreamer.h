//
//  JsonStreamer.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 26/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__JsonStreamer__
#define __G_Ear_core__JsonStreamer__

#include "stdplus.h"
#include "json-forwards.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    class IDownloader;
    
    class JsonStreamer
    {
    public:
        JsonStreamer(const shared_ptr<IDownloader> &downloader);
        bool nextChunk(Json::Value &chunk);
        
    private:
        const shared_ptr<IDownloader> _downloader;
        size_t _position;
        std::string _bufferHtml;
    };
}

#endif /* defined(__G_Ear_core__JsonStreamer__) */
