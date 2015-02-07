//
//  CocoaWebWindow.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/4/13.
//
//

#ifndef __G_Ear_Player__CocoaWebWindow__
#define __G_Ear_Player__CocoaWebWindow__

#include <WebKit/WebKit.h>
#include "IWebWindow.h"

@class CocoaWebDelegate;

namespace Gear
{
    class CocoaWebWindow final : public IWebWindow
    {
    public:
        CocoaWebWindow(const shared_ptr<IWebWindowDelegate> &delegate);
        ~CocoaWebWindow();
        virtual void setUserAgent(const string &userAgent);
        virtual void loadUrl(const string &url);
        virtual void injectCookie(const WebCookie &cookie);
        virtual vector<WebCookie> cookies();
        virtual void show(const string &title);
        virtual string url() const;
        virtual bool visible() const;
        virtual string fieldValueForElementId(const string &elementId) const;
        virtual string title() const;
        
        // implementation details, do not call these!
        void didFailLoadWithError(NSError *error);
        void didFinishLoad();
        void navigationAction();
        void setUrl(const string &);
        
    private:
        string _url;
        id _webView;
        NSWindow *_webWindow;
        CocoaWebDelegate *_webDelegate;
    };
}

#endif /* defined(__G_Ear_Player__CocoaWebWindow__) */
