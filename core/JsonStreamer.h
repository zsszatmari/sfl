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
    using std::string;
    using std::function;

    class IDownloader;
    
    class JsonStreamer
    {
    public:
        JsonStreamer();
        void gotData(const string &data, const function<void(const Json::Value &)> &gotChunk);

        static const int ChunkSize;
        
    private:
        std::string _bufferHtml;
        //int _scannedEnd;
    };
}

#endif /* defined(__G_Ear_core__JsonStreamer__) */
