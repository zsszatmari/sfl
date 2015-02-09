//
//  CocoaWebWindow.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/4/13.
//
//

#include "CocoaWebWindow.h"
#include "StringUtility.h"
#import "IGIsolatedCookieWebView.h"

using namespace Gear;

@interface CocoaWebDelegate : NSObject {
    CocoaWebWindow *_webWindow;
}

- (id)initWithWebWindow:(CocoaWebWindow *)webWindow;

@end

@implementation CocoaWebDelegate

- (id)initWithWebWindow:(CocoaWebWindow *)webWindow
{
    self = [super init];
    if (self) {
        _webWindow = webWindow;
    }
    return self;
}

- (void)webView:(WebView *)sender didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    if ([error domain] == NSURLErrorDomain && [error code] == -999) {
        // user cancelled
        return;
    }
    
    _webWindow->didFailLoadWithError(error);
}

- (void)webView:(WebView *)sender didFailProvisionalLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    _webWindow->didFailLoadWithError(error);
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    _webWindow->didFinishLoad();
}

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id < WebPolicyDecisionListener >)listener
{
    NSString *url = [[request URL] absoluteString];
    _webWindow->setUrl(convertString(url));
    NSLog(@"url: %@", url);
    
    if ([url hasPrefix:@"http://localhost"]) {
        NSLog(@"ignore");
        
        [listener ignore];
        _webWindow->didFinishLoad();
    } else {
        NSLog(@"use");
        
        _webWindow->navigationAction();
        [listener use];
    }
}


@end

namespace Gear
{
#define method CocoaWebWindow::
    
    static WebView *createWebView(bool controlled, bool standalone)
    {
        if (standalone) {
            return nil;
        }
        if (controlled) {
            return [[IGIsolatedCookieWebView alloc] init];
        } else {
            return [[WebView alloc] init];
        }
    }
    
    method CocoaWebWindow(const shared_ptr<IWebWindowDelegate> &delegate) :
        IWebWindow(delegate),
        _webView(createWebView(delegate->controlledCookies(), delegate->allowStandaloneBrowser())),
        _webDelegate([[CocoaWebDelegate alloc] initWithWebWindow:this])
    {
        NSString *prefIdentifier = @"GoogleMusicWebPrefs";
        [_webView setPreferencesIdentifier:prefIdentifier];
        // if we use this, not even the google logo will show!
        //[[_webView preferences] setLoadsImagesAutomatically:NO];
        
        [_webView setFrameLoadDelegate:_webDelegate];
        [_webView setPolicyDelegate:_webDelegate];
    }
    
    method ~CocoaWebWindow()
    {
        id webView = _webView;
        [webView setFrameLoadDelegate:nil];
        [webView setPolicyDelegate:nil];
        id webWindow = _webWindow;
        @autoreleasepool {
            dispatch_async(dispatch_get_main_queue(), ^{
                [webView close];
                [webWindow close];
            });
        }
    }
    
    void method setUserAgent(const string &userAgent)
    {
        [_webView setCustomUserAgent:convertString(userAgent)];
    }
    
    void method loadUrl(const string &url)
    {
        if (_webView) {
            [_webView setMainFrameURL:convertString(url)];
        } else {
            [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:convertString(url)]];
        }
    }
    
    void method injectCookie(const WebCookie &cookie)
    {
        NSHTTPCookie *httpCookie = [NSHTTPCookie cookieWithProperties:convertStringMap(cookie.properties())];
        
        [_webView injectCookie:httpCookie];
    }
    
    vector<WebCookie> method cookies()
    {
        NSArray *httpCookies = [_webView getCookies];
        vector<WebCookie> ret;
        for (NSHTTPCookie *cookie in httpCookies) {
            ret.push_back(WebCookie(convertStringMap([cookie properties])));
        }
        return ret;
    }
    
    void method didFinishLoad()
    {
        _delegate->didFinishLoad();
    }
    
    void method didFailLoadWithError(NSError *error)
    {
        _delegate->didFailLoad(convertString([error localizedDescription]));
    }
    
    void method navigationAction()
    {
        _delegate->navigationAction();
    }
    
    void method show(const string &title)
    {
        if (_webView == nil) {
            // standalone mode
            return;
        }
        
        if (_webWindow == nil) {
            // a width of 320 is too little for a nice youtube login
            // 440 is also too little for youtube channel creation... 510 needed
            _webWindow = [[NSWindow alloc] initWithContentRect:CGRectMake(100,300,510,500) styleMask:NSTitledWindowMask | NSResizableWindowMask | NSClosableWindowMask backing:NSBackingStoreBuffered defer:NO];
        }
        [_webWindow setReleasedWhenClosed:NO];
        [_webWindow setTitle:convertString(title)];
        [_webWindow setContentView:_webView];
        [_webWindow makeKeyAndOrderFront:nil];
        
        WebView *webView = _webView;
        [[webView preferences] setLoadsImagesAutomatically:YES];
    }
    
    bool method visible() const
    {
        return [_webWindow isVisible];
    }
    
    string method fieldValueForElementId(const string &elementId) const
    {
        WebFrame *frame = [_webView mainFrame];
        DOMHTMLDocument *document = (DOMHTMLDocument *)[frame DOMDocument];
        DOMHTMLInputElement *emailField = (DOMHTMLInputElement *)[document getElementById:convertString(elementId)];
        
        if (emailField != nil) {
            return convertString([emailField value]);
        }
        return "";
    }
    
    string method url() const
    {
        return _url;
    }
    
    void method setUrl(const string &url)
    {
        _url = url;
    }
    
    string method title() const
    {
        return convertString([_webView mainFrameTitle]);
    }
}