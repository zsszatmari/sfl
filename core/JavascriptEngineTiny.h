//
//  JavascriptEngineTiny.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 07/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__JavascriptEngineTiny__
#define __G_Ear_core__JavascriptEngineTiny__

#include "IJavascriptEngine.h"
#include "duktape.h"

namespace Gear
{
    class JavascriptEngineTiny : public IJavascriptEngine
    {
    public:
    	JavascriptEngineTiny();
        virtual ~JavascriptEngineTiny();

        virtual void load(const std::string &js);
        virtual std::string execute(const std::string &statement);
        
    private:
    	duk_context *_ctx;
    };
}

#endif /* defined(__G_Ear_core__JavascriptEngineTiny__) */
