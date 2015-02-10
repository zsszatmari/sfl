//
//  InAppManager.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 11/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>
#include "stdplus.h"

@interface InAppStore : NSObject<SKPaymentTransactionObserver,SKProductsRequestDelegate>

+ (InAppStore *)sharedStore;
- (BOOL)isAvailable:(NSString *)identifier;
- (BOOL)isInProgress:(NSString *)identifier;
- (void)tryPurchase:(NSString *)identifier success:(dispatch_block_t)success;
- (void)refresh;
- (void)restorePurchases;
+ (BOOL)useServer;

@end
