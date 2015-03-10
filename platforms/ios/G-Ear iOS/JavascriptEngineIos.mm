//
//  JavascriptEngineIos.cpp
//  jsdecrypt
//
//  Created by Zsolt Szatmari on 16/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "JavascriptEngineIos.h"

using namespace std;

@interface JavascriptHelper : NSObject<UIWebViewDelegate>
@end

@implementation JavascriptHelper {
    Gear::JavascriptEngineIos::State *_state;
}

- (id)initWithState:(Gear::JavascriptEngineIos::State *)state
{
    self = [super init];
    if (self) {
        _state = state;
    }
    return self;
}

- (void)webViewDidFinishLoad:(UIWebView *)webView
{
    _state->loaded();
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
    _state->loaded();
}

@end

namespace Gear
{
#define method JavascriptEngineIos::
    
    method State::State() :
        _loaded(false)
    {
    }
    
    void method State::loaded()
    {
        std::lock_guard<mutex> l(_loadedMutex);
        _loaded = true;
        _loadedVariable.notify_all();
    }
    
    static void executeSync(dispatch_block_t block)
    {
        assert(![NSThread isMainThread]);
        dispatch_sync(dispatch_get_main_queue(), block);
    }
    
    method JavascriptEngineIos()
    {
    }
    
    method ~JavascriptEngineIos()
    {
        executeSync(^{
            _webView = nil;
        });
    }
    
    void method load(const std::string &js)
    {
        @autoreleasepool {
            
            NSString *html = [NSString stringWithFormat:@"<html><body><script>%s</script></body></html>", js.c_str()];
            
            executeSync(^{
                _webView = [[UIWebView alloc] initWithFrame:CGRectMake(0, 0, 1, 1)];
                _helper = [[JavascriptHelper alloc] initWithState:&_state];
                [_webView setDelegate:_helper];
                [_webView loadHTMLString:html baseURL:nil];
            });
        }
    }
    
    std::string method execute(const std::string &statement)
    {
        @autoreleasepool {
            {
                unique_lock<mutex> l(_state._loadedMutex);
                while (!_state._loaded) {
                    _state._loadedVariable.wait(l);
                }
            }
            
            __block NSString *result;
            executeSync(^{
                result = [_webView stringByEvaluatingJavaScriptFromString:[NSString stringWithCString:statement.c_str() encoding:NSUTF8StringEncoding]];
            });
            return [result cStringUsingEncoding:NSUTF8StringEncoding];
        }
    }
}
