//
//  LicenseManager.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 11/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef G_Ear_core_LicenseManager_h
#define G_Ear_core_LicenseManager_h

// this file is intentionally header only

#if TARGET_OS_IPHONE
#include "Apple/receigen.h"
#include "IDelegate.h"
#import "InAppStore.h"
#include "StringUtility.h"
#endif
using std::function;

/*
static inline bool checkLicenseImmediately(const Gear::IService &service)
{
    if (!service.inAppNecessary()) {
        return true;
    }
    
    @autoreleasepool {
        __block bool success = false;
        
        NSString *identifier = Gear::convertString("com.treasurebox.gear.ios." + service.inAppIdentifier());
        ReceiptValidation_CheckInAppPurchases(@[identifier], ^(NSString *identifier, BOOL isPresent, NSDictionary *purchaseInfo) {
            
            if (isPresent) {
                success = true;
            } else {
            }
        });
        return success;
    }
}*/ 

static inline void checkLicense(const Gear::IService &service, const function<void()> &aSuccess , const function<void()> &aFail)
{
#if TARGET_OS_IPHONE
    
#ifdef DEBUG
//#define DISABLE_RECEIPT_REFRESH_ON_FAIL
#endif

    @autoreleasepool {
        NSString *identifier = Gear::convertString("com.treasurebox.gear.ios." + service.inAppIdentifier());
        
        if ([InAppStore useServer]) {
            
            if ([[InAppStore sharedStore] isAvailable:identifier]) {
                aSuccess();
            } else {
                aFail();
            }
            return;
        }
        
        // otherwise dumb objc blocks dont retain
        auto success = aSuccess;
        auto fail = aFail;
        
        if (!service.inAppNecessary()) {
            success();
            return;
        }
    
        __block BOOL called = NO;
        
        
        ReceiptValidation_CheckInAppPurchases(@[identifier], ^(NSString *identifier, BOOL isPresent, NSDictionary *purchaseInfo) {
            
            called = YES;
            if (isPresent) {
                success();
            } else {
                fail();
            }
        });
        if (!called) {
#ifndef DISABLE_RECEIPT_REFRESH_ON_FAIL
#if TARGET_OS_IPHONE
            Gear::icheck(Gear::convertString(identifier), [=]{
                __block BOOL calledIn = NO;
                ReceiptValidation_CheckInAppPurchases(@[identifier], ^(NSString *identifier, BOOL isPresent, NSDictionary *purchaseInfo) {
                    
                    calledIn = YES;
                    if (isPresent) {
                        success();
                    } else {
                        fail();
                    }
                });
                if (!calledIn) {
                    fail();
                }

            }, fail);
#else
            fail();
#endif
#else
            fail();
#endif
            
        }
    }
    
#else
    aSuccess();
#endif
}



#endif
