//
//  YouTubeSignature.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 16/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__YouTubeSignature__
#define __G_Ear_core__YouTubeSignature__

#include "stdplus.h"
#include "AtomicPtr.h"

namespace Gear
{
    class IJavascriptEngine;
    
    class YouTubeDecode final
    {
    public:
        shared_ptr<IJavascriptEngine> engine(const std::string &playerUrl);
	    shared_ptr<IJavascriptEngine> createEngine(const std::string &playerJs);
        
    private:
        AtomicPtr<std::pair<std::string,shared_ptr<IJavascriptEngine>>> _currentEngine;
    };
}

#endif /* defined(__G_Ear_core__YouTubeSignature__) */
