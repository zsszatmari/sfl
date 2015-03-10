//
//  JavascriptEngineIos.h
//  jsdecrypt
//
//  Created by Zsolt Szatmari on 16/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __jsdecrypt__JavascriptEngineIos__
#define __jsdecrypt__JavascriptEngineIos__

#include <UIKit/UIKit.h>
#include <mutex>
#include "IJavascriptEngine.h"

@class JavascriptHelper;

namespace Gear
{
    class JavascriptEngineIos final : public IJavascriptEngine
    {
    public:
        JavascriptEngineIos();
        virtual ~JavascriptEngineIos();
        virtual void load(const std::string &js);
        virtual std::string execute(const std::string &statement);
        
        class State final
        {
        public:
            void loaded();
            
        private:
            State();
            
            bool _loaded;
            std::mutex _loadedMutex;
            std::condition_variable _loadedVariable;
            
            friend class JavascriptEngineIos;
        };
        
    private:
        
        State _state;
        
        UIWebView *_webView;
        JavascriptHelper *_helper;
    };
}

#endif /* defined(__jsdecrypt__JavascriptEngineIos__) */
