//
//  IWebWindow.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/4/13.
//
//

#ifndef __G_Ear_Player__IWebWindow__
#define __G_Ear_Player__IWebWindow__

#include <string>
#include <vector>
#include "stdplus.h"
#include "WebCookie.h"
#include "IWebWindowDelegate.h"
#include "GearUtility.h"
#include MEMORY_H

namespace Base
{
    class Timer;
}

namespace Gear
{
    using std::string;
    using std::vector;
    
    class core_export IWebWindow
    {
    public:
        IWebWindow(const shared_ptr<IWebWindowDelegate> &delegate);
        virtual ~IWebWindow();
        virtual void setUserAgent(const string &userAgent) = 0;
        virtual string url() const = 0;
        virtual string title() const = 0;
        virtual vector<WebCookie> cookies() = 0;
        virtual void loadUrl(const string &url) = 0;
        virtual void injectCookie(const WebCookie &cookie) = 0;
        virtual void show(const string &title) = 0;
        virtual bool visible() const = 0;
        virtual string fieldValueForElementId(const string &elementId) const = 0;
        static void checkPeriodically(const shared_ptr<IWebWindow> &webWindow);
        
    protected:
        const shared_ptr<IWebWindowDelegate> _delegate;

    private:
        shared_ptr<Base::Timer> _timer;
    };
}

#endif /* defined(__G_Ear_Player__IWebWindow__) */
