//
//  IJavascript.h
//  jsdecrypt
//
//  Created by Zsolt Szatmari on 16/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __jsdecrypt__IJavascript__
#define __jsdecrypt__IJavascript__

#include <string>

namespace Gear
{
    class IJavascriptEngine
    {
    public:
        virtual ~IJavascriptEngine();
        
        // these must not be called from the main thread
        virtual void load(const std::string &js) = 0;
        virtual std::string execute(const std::string &statement) = 0;
    };
}

#endif /* defined(__jsdecrypt__IJavascript__) */
