//
//  CocoaTouchWebWindow.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/5/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__CocoaTouchWebWindow__
#define __G_Ear_iOS__CocoaTouchWebWindow__

#import "IWebWindow.h"

@class WebViewController;
@class CocoaTouchWebDelegate;

namespace Gear
{    
    class CocoaTouchWebWindow final : public IWebWindow
    {
    public:
        CocoaTouchWebWindow(const shared_ptr<IWebWindowDelegate> &delegate);
        virtual ~CocoaTouchWebWindow();
        
        virtual void setUserAgent(const string &userAgent);
        virtual vector<WebCookie> cookies();
        virtual void loadUrl(const string &url);
        virtual string url() const;
        virtual void injectCookie(const WebCookie &cookie);
        virtual void show(const string &title);
        virtual bool visible() const;
        virtual string fieldValueForElementId(const string &elementId) const;
        virtual string title() const;
        
        void setTitle(const string &title);
        
    private:
        UIWebView *_webView;
        CocoaTouchWebDelegate *_webDelegate;
        WebViewController *_webController;
        shared_ptr<bool> _shouldShow;
        string _title;
        string _url;
    };
}

#endif /* defined(__G_Ear_iOS__CocoaTouchWebWindow__) */
