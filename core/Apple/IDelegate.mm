//
//  IDelegate.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 19/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <StoreKit/StoreKit.h>
#include "IDelegate.h"
#include "InAppStore.h"

using std::function;

@interface IDelegate : NSObject<SKRequestDelegate>

@property (nonatomic,strong) id selfRetain;
@property (nonatomic,assign) function<void()> success;
@property (nonatomic,assign) function<void()> fail;


@end

@implementation IDelegate

- (id)init
{
    self = [super init];
    if (self) {
        self.selfRetain = self;
    }
    return self;
}

- (void)requestDidFinish:(SKRequest *)request
{
    dispatch_async(dispatch_get_main_queue(), ^{
        auto s = self.success;
        s();
        self.selfRetain = nil;
        [[InAppStore sharedStore] refresh];
    });
}

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
    dispatch_async(dispatch_get_main_queue(), ^{
        auto f = self.fail;
        f();
        self.selfRetain = nil;
    });
}

@end

namespace Gear
{
    void icheck(const std::string &identifier, const function<void()> &success , const function<void()> &fail)
    {
#if TARGET_OS_IPHONE
        // this is only for the case when we are delayed
        if (floor(NSFoundationVersionNumber) > NSFoundationVersionNumber_iOS_6_1) {
            // iOS 7 or later
            NSURL *receipt = [[NSBundle mainBundle] appStoreReceiptURL];
            if (![[NSFileManager defaultManager] fileExistsAtPath:[receipt path]]) {
                SKReceiptRefreshRequest *request = [[SKReceiptRefreshRequest alloc] init];
                IDelegate *del = [[IDelegate alloc] init];
                del.success = success;
                del.fail = fail;
                [request setDelegate:del];
                [request start];
            } else {
                fail();
            }
        } else {
            fail();
        }
#else
        fail();
#endif
    }
}