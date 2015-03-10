//
//  CocoaTouchWebWindow.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/5/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <iostream>
#include "CocoaTouchWebWindow.h"
#include "StringUtility.h"
#import "AppDelegate.h"
#import "WebViewController.h"
#include "Debug.h"

using namespace Gear;

@interface CocoaTouchWebDelegate : NSObject<UIWebViewDelegate> {
    CocoaTouchWebWindow *_webWindow;
    shared_ptr<IWebWindowDelegate> _delegate;
    UIViewController *_webController;
    BOOL shouldShow;
}

- (id)initWithWebWindow:(CocoaTouchWebWindow *)webWindow delegate:(const shared_ptr<IWebWindowDelegate>)delegate;

@property(nonatomic, strong) UIViewController *webController;

@end

namespace Gear
{
#define method CocoaTouchWebWindow::
    
    method CocoaTouchWebWindow(const shared_ptr<IWebWindowDelegate> &delegate) :
        IWebWindow(delegate),
        _webView([[UIWebView alloc] init]),
        _webDelegate([[CocoaTouchWebDelegate alloc] initWithWebWindow:this delegate:delegate]),
        _shouldShow(new bool(false))
    {
        [_webView setDelegate:_webDelegate];
        
#ifndef DEBUG_DONT_CLEAR_LOGIN_COOKIES
        // clear cookies
        NSHTTPCookieStorage *cookies = [NSHTTPCookieStorage sharedHTTPCookieStorage];
        for (id cookie in [cookies cookies]) {
            [cookies deleteCookie:cookie];
        }
#endif
    }
    
    method ~CocoaTouchWebWindow()
    {
        [_webView setDelegate:nil];
        
        *_shouldShow = false;
#if DEBUG
        NSLog(@"hide web window");
#endif
        
        // hide
        [_webController dismissViewControllerAnimated:YES completion:nil];
    }
    
    void method setUserAgent(const string &userAgent)
    {
        NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:convertString(userAgent), @"UserAgent", nil];
        [[NSUserDefaults standardUserDefaults] registerDefaults:dictionary];
    }
    
    vector<WebCookie> method cookies()
    {
        NSHTTPCookieStorage *httpCookies = [NSHTTPCookieStorage sharedHTTPCookieStorage];
        
        vector<WebCookie> ret;
        for (NSHTTPCookie *cookie in [httpCookies cookies]) {
            ret.push_back(WebCookie(convertStringMap([cookie properties])));
        }
        return ret;
    }

    void method injectCookie(const WebCookie &aCookie)
    {
        NSHTTPCookie *cookie = [NSHTTPCookie cookieWithProperties:convertStringMap(aCookie.properties())];
        NSHTTPCookieStorage *httpCookies = [NSHTTPCookieStorage sharedHTTPCookieStorage];
        
        [httpCookies setCookie:cookie];
    }
    
    void method loadUrl(const string &url)
    {
        _url = url;
        if (_delegate && _delegate->allowStandaloneBrowser()) {
        } else {
            [_webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:convertString(url)]]];
        }
    }
    
    void method show(const string &title)
    {
        if (_delegate && _delegate->allowStandaloneBrowser()) {
            [[UIApplication sharedApplication] openURL:[NSURL URLWithString:convertString(_url)]];
            return;
        }
        
        UIStoryboard *storyboard = [[AppDelegate sharedDelegate] storyBoard];
        _webController = [storyboard instantiateViewControllerWithIdentifier:@"webController"];
        [_webController setWebView:_webView];
        [_webDelegate setWebController:_webController];

        UIButton *cancel_butt = [UIButton buttonWithType: UIButtonTypeCustom];
        [cancel_butt setTitleColor: [UIColor colorWithRed: 66. / 255. green: 127. / 255. blue: 237. / 255. alpha: 1.] forState: UIControlStateNormal];
        cancel_butt.frame = CGRectMake(0, 17, 80, 22);
        [cancel_butt setTitle: @"Back" forState: UIControlStateNormal];
        cancel_butt.titleLabel.font = [UIFont fontWithName: @"Helvetica" size: 15];
        [cancel_butt addTarget: _webDelegate action: @selector(cancel_action:) forControlEvents: UIControlEventTouchUpInside];
        [_webView.scrollView addSubview: cancel_butt];

        *_shouldShow = true;
        auto shouldShow = _shouldShow;
        auto controller = _webController;
        
        [[AppDelegate sharedDelegate].topViewController presentViewController:_webController animated:YES completion:^{
            
            if (*shouldShow == false) {
                [controller dismissViewControllerAnimated:YES completion:nil];
            }
        }];
        
#if DEBUG
        NSLog(@"show web window");
#endif
        
    }
    
    bool method visible() const
    {
        return _webController != nil;
    }
         
    string method fieldValueForElementId(const string &elementId) const
    {
        NSString *script = [NSString stringWithFormat:@"document.getElementById('%@').value", convertString(elementId)];
        NSString *ret = [_webView stringByEvaluatingJavaScriptFromString:script];
                         
        return convertString(ret);
    }

    string method url() const
    {
        return convertString([[[_webView request] URL] absoluteString]);
    }
    
    void method setTitle(const string &title)
    {
        _title = title;
    }
    
    string method title() const
    {
        return _title;
    }
}

@implementation CocoaTouchWebDelegate

- (id)initWithWebWindow:(CocoaTouchWebWindow *)webWindow delegate:(const shared_ptr<IWebWindowDelegate>)delegate;
{
     self = [super init];
     if (self) {
         _webWindow = webWindow;
         _delegate = delegate;
     }
     return self;
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
    if (_delegate) {
        _delegate->navigationAction();
    }
    return YES;
}

- (void)webViewDidFinishLoad:(UIWebView *)webView
{
    NSString *title = [webView stringByEvaluatingJavaScriptFromString:@"document.title"];
    _webWindow->setTitle(convertString(title));
    
    if (_delegate) {
        _delegate->didFinishLoad();
    }
    
    [webView stringByEvaluatingJavaScriptFromString:@"document.body.style.webkitTouchCallout='none'; document.body.style.KhtmlUserSelect='none'"];
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
    if (_delegate) {
        _delegate->didFailLoad(convertString([error localizedDescription]));
    }
}

- (void) cancel_action: (id) sender
{
    if (_delegate) {
        _delegate -> didFailLoad("");
    } else {
        [self.webController dismissViewControllerAnimated:YES completion:nil];
    }
}

@end
