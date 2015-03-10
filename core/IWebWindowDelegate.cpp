//
//  IWebWindowDelegate.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/4/13.
//
//

#include "IWebWindowDelegate.h"
#include "IApp.h"

namespace Gear
{
#define method IWebWindowDelegate::
 
    using std::function;

    method ~IWebWindowDelegate()
    {
    }
    
    bool method controlledCookies() const
    {
        return true;
    }
    
    bool method allowStandaloneBrowser() const
    {
        return false;
    }

    MEMORY_NS::shared_ptr<IWebWindow> method create(const std::function<void(const MEMORY_NS::shared_ptr<IWebWindow> &, const std::function<void()> &dispose)> &finish, const std::function<void(const MEMORY_NS::shared_ptr<IWebWindow> &w, const std::function<void()> &dispose)> &fail, const std::function<void(const shared_ptr<IWebWindow> &w, const std::function<void()> &dispose)> &navigation)
    {
        struct FunctionalDelegate : public IWebWindowDelegate
        {
            virtual void didFailLoad(const string &errorMessage) override
            {
                _fail(_window, _dispose);
            }

            virtual void didFinishLoad() override
            {
                _finish(_window, _dispose);
            }

            virtual void navigationAction() override
            {
                _navigation(_window, _dispose);
            }

            virtual bool controlledCookies() const
            {
                return false;
            }

            function<void(const shared_ptr<IWebWindow> &, const function<void()> &dispose)> _fail;
            function<void(const shared_ptr<IWebWindow> &, const function<void()> &dispose)> _finish;
            function<void(const shared_ptr<IWebWindow> &, const function<void()> &dispose)> _navigation;
            shared_ptr<IWebWindow> _window;
            function<void()> _dispose;
        };
        
        shared_ptr<FunctionalDelegate> ret(new FunctionalDelegate());
        ret->_fail = fail;
        ret->_finish = finish;
        ret->_navigation = navigation;
        ret->_dispose = [ret]{
            ret->_window.reset();
        };
        auto window = IApp::instance()->createWebWindow(ret);
        ret->_window = window;
        return window;
    }
}