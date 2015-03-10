//
//  WebViewController.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/5/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "WebViewController.h"

@implementation WebViewController

- (void)setWebView:(UIWebView *)webView
{
    if (webView != nil) {
        [webView setFrame:self.view.bounds];
        webView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        [self.view addSubview:webView];
    }
}

@end
