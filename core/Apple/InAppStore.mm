//
//  InAppManager.mm
//  G-Ear core
//
//  Created by Zsolt Szatmari on 11/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#import "InAppStore.h"
#include "receigen.h"
#include "json.h"
#include "Encoder.h"
#include "HttpDownloader.h"
#import "StringUtility.h"
#include "AppleHttpDownloader.h"
#include "CoreDebug.h"

using namespace Recd;
using namespace Gear;
using std::string;

@implementation InAppStore {
    NSMutableArray *available;
    NSArray *possible;
    NSMapTable *alertViewsToProducts;
    NSMutableDictionary *productToSuccess;
    NSMutableArray *inProgress;
#if TARGET_OS_IPHONE
    UIAlertView *generalAlert;
#endif
    int restoreCount;
}

static NSArray *productIdentifiers;
static NSArray * const productIdentiferSuffixes = @[@"googlemusic",@"youtube" /*,@"offline"*/];

+ (BOOL)useServer
{
#if TARGET_OS_IPHONE
    if (floor(NSFoundationVersionNumber) <= NSFoundationVersionNumber_iOS_6_1) {
        return true;
    }
#else
    return false;
#endif
    // for testing
    // return true;
    
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *bundleId = [bundle bundleIdentifier];
    if ([bundleId isEqualToString:@"com.treasurebox.gear.ios"]) {
        return NO;
    }
    
    if ([bundleId hasPrefix:@"com.treasurebox.gear.ios"]) {
        // cheating
        return NO;
    }
    
    return YES;
}

+ (InAppStore *)sharedStore
{
    static dispatch_once_t onceToken;
    static InAppStore *store = nil;
    dispatch_once(&onceToken, ^{
        NSMutableArray *array = [NSMutableArray array];
        for (NSString *suffix in productIdentiferSuffixes) {
            [array addObject:[NSString stringWithFormat:@"%@.%@", [[NSBundle mainBundle] bundleIdentifier], suffix]];
        }
        productIdentifiers = array;
        store = [[InAppStore alloc] init];
    });
    return store;
}

- (id)init
{
    self = [super init];
    if (self) {
        inProgress = [NSMutableArray array];
        alertViewsToProducts = [[NSMapTable alloc] initWithKeyOptions:NSMapTableStrongMemory valueOptions:NSMapTableStrongMemory capacity:5];
        productToSuccess = [NSMutableDictionary dictionary];
        
        [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
        
        // shouldn't refresh receipt here, it would cause asking for password when going to preferences
        
        SKProductsRequest *productsRequest = [[SKProductsRequest alloc]
                                              initWithProductIdentifiers:[NSSet setWithArray:productIdentifiers]];
        productsRequest.delegate = self;
        [productsRequest start];
    }
    return self;
}

- (void)refresh
{
    available = nil;
}

- (void)productsRequest:(SKProductsRequest *)request
     didReceiveResponse:(SKProductsResponse *)response
{
    possible = response.products;
    
    for (NSString *invalidIdentifier in response.invalidProductIdentifiers) {
#ifdef DEBUG
        NSLog(@"invalid iap identifier: %@", invalidIdentifier);
#endif
    }
}

static string deviceIdentifier()
{
#if TARGET_OS_IPHONE
    NSUUID *uuid = [UIDevice currentDevice].identifierForVendor;
    NSString *str = [uuid UUIDString];
    return convertString(str);
#else
    return "0";
#endif
}

static const std::string pubkey("MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyzVZbp7Dvmjm+TGBKAsznXELQnv9F8VHguowpALN6Tc98ySxRrRlIbq5oeDa9skSplbpYtiE9Svtaq4WWtcdBTAGPaUyJ11ZEvrAmxA9pfm9AXM0LaWMxnzyTWhy9UxyW/dWUnJlu7BzW4EiU66zGU3qrGxqRiHn407pDPJl7W/wrWAMR6/XCDTgjHmLzmzmOfrH+557vXiJ4RnqYWtSo3q6z45E78tRljTSMG809286VA6FNSDhCh4SoqYd7Ye0qK1Pc+AO3TeTALG3NroCr+BEJAWV7MhsQ3sne3DFAxMLRLquPeaXKvChSJb9avIfU72S1zBBRtw2eaq5lGCQNwIDAQAB");

static bool verifyProcessedReceipt(const string &processedReceipt, const string &productId)
{
    Json::Reader reader;
    Json::Value replyValue;
    reader.parse(processedReceipt, replyValue);
    
    auto payloadCoded = replyValue.get("payload","").asString();
    auto signature = replyValue.get("signature","").asString();
    if (!Encoder::verifySignature(payloadCoded, Encoder::base64decode(pubkey), signature)) {
        return false;
    }
    
    auto payloadV = Encoder::base64decode(payloadCoded);;
    string payload(payloadV.begin(), payloadV.end());
    
    Json::Value payloadJson;
    reader.parse(payload, payloadJson);
    
    if (payloadJson.get("device-identifier","").asString() != deviceIdentifier()) {
        return false;
    }
    auto receipt = payloadJson.get("receipt", Json::objectValue);
    if (payloadJson.get("status",-1).asInt() != 0) {
        return false;
    }
    auto receiptProductId = receipt.get("product_id","").asString();
    if (receiptProductId != productId) {
        return false;
    }
    
    //std::cout << "got reply: " << payload << std::endl;
    return true;
}

- (BOOL)isAvailable:(NSString *)identifierToCheck
{
    if (![productIdentifiers containsObject:identifierToCheck]) {
        return YES;
    }
    if ([[self class] useServer]) {
        NSString *str = [[NSUserDefaults standardUserDefaults] objectForKey:identifierToCheck];
        if (!str) {
            return NO;
        }
        return verifyProcessedReceipt(convertString(str), convertString(identifierToCheck));
    } else {
        if (!available) {
            available = [NSMutableArray array];
            
            if (![[self class] useServer]) {
#ifdef DEBUG_INAPP_ENABLE_EVERYTHING
                NSLog(@"Warning!!!!!");
                [available addObjectsFromArray:productIdentifiers];
#else
                ReceiptValidation_CheckInAppPurchases(productIdentifiers, ^(NSString *identifier, BOOL isPresent, NSDictionary *purchaseInfo) {
                    
                    if (isPresent) {
                        [available addObject:identifier];
                    }
                });
#endif
            }
        }
        
        return [available containsObject:identifierToCheck];
    }
}

- (BOOL)isInProgress:(NSString *)identifier
{
    return [inProgress containsObject:identifier];
}

- (void)tryPurchase:(NSString *)identifier success:(dispatch_block_t)success
{
    if (![inProgress containsObject:identifier]) {
        [inProgress addObject:identifier];
    }
    
    if (!possible) {
        double delayInSeconds = 1.0;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
            [self tryPurchase:identifier success:success];
        });
        return;
    }
    
    SKProduct *product = nil;
    for (SKProduct *it in possible) {
        if ([[it productIdentifier] isEqual:identifier]) {
            product = it;
            break;
        }
    }
    if (!product) {
        [inProgress removeObject:identifier];
        return;
    }
    
    [productToSuccess setObject:success forKey:[product productIdentifier]];
    
    NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
    [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
    [numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
    [numberFormatter setLocale:product.priceLocale];
    NSString *formattedPrice = [numberFormatter stringFromNumber:product.price];
    
#if TARGET_OS_IPHONE
    NSString *message = [NSString stringWithFormat:@"%@\nDo you wish to buy it for %@?", [product localizedDescription], formattedPrice];
#if DEBUG
    NSLog(@"inapp message: %@", message);
#endif
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:[product localizedTitle] message:message delegate:self cancelButtonTitle:@"No" otherButtonTitles:@"Yes", nil];
    [alertView show];
    [alertViewsToProducts setObject:product forKey:alertView];
#endif
}

#if TARGET_OS_IPHONE
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    SKProduct *product = [alertViewsToProducts objectForKey:alertView];
    [alertViewsToProducts removeObjectForKey:alertView];
    
    if (buttonIndex == 0) {
        // cancel
        [inProgress removeObject:[product productIdentifier]];
        return;
    }
    SKPayment *payment = [SKPayment paymentWithProduct:product];
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}
#endif

- (void)checkReceipt:(NSData *)receipt forProduct:(NSString *)product andThen:(void(^)(BOOL))finish
{
    Json::Value value;
    
    value["receipt-data"] = Encoder::base64encode(std::vector<unsigned char>(static_cast<const unsigned char*>([receipt bytes]), static_cast<const unsigned char*>([receipt bytes]) + [receipt length]));
    
    NSBundle *bundle = [NSBundle mainBundle];
    NSDictionary *infoDict = [bundle infoDictionary];
    value["bundle-version"] = [[infoDict objectForKey:@"CFBundleVersion"] longLongValue];
    value["bundle-id"] = Gear::convertString([bundle bundleIdentifier]);
    value["device-identifier"] = deviceIdentifier();

    // does not need a particularly strong key, since there is only one chance for even trying to access
    auto keys = Encoder::generatePublicPrivateKeys(512);
    value["client-key"] = Encoder::base64encode(keys.first);
    
    Json::FastWriter writer;
    auto message = Encoder::seal(writer.write(value), Encoder::base64decode(pubkey));
#ifdef DEBUG
    //const string url = "http://192.168.1.112:8000/verify";
    const string url = "http://receipts.treasurebox.hu/verify";
#else
    const string url = "http://receipts.treasurebox.hu/verify";
#endif
    //auto downloader = HttpDownloader::create(url, std::map<string,string>(), "POST", message);
    // TODO: there is a problem with GenericHttpDownloader which can have other consequences, this is the starting point to debug
    auto downloader = AppleHttpDownloader::create(url, std::map<string,string>(), "POST", message);
    downloader->waitUntilFinished();
    
    void(^signalError)(NSString *) = ^(NSString *error){
        dispatch_async(dispatch_get_main_queue(),^{
#if TARGET_OS_IPHONE
            generalAlert = [[UIAlertView alloc]
                            initWithTitle:@"Error"
                            message:error
                            delegate:nil
                            cancelButtonTitle:@"Dismiss"
                            otherButtonTitles:nil];
            [generalAlert show];
#endif
        });
    };
    
    if (downloader->failed()) {
        signalError(@"Error connecting to server.\nPlease retry later!");
        finish(NO);
    } else {
        string codedReply = *downloader;
        string processedReceipt = Encoder::open(codedReply, keys.second);
        
        auto productId = convertString(product);
        bool okay = verifyProcessedReceipt(processedReceipt, productId);
        
        if (okay) {
            [[NSUserDefaults standardUserDefaults] setObject:convertString(processedReceipt) forKey:product];
            finish(YES);
        } else {
            signalError(@"Error processing transaction.\nPlease retry later!");
            finish(NO);
        }
    }
}

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions
{
// "Important: Once the transaction is finished, Store Kit can not tell you that this item is already purchased. It is important that applications process the transaction completely before calling finishTransaction:."
    
    for (SKPaymentTransaction *transaction in transactions) {
        switch(transaction.transactionState) {
            case SKPaymentTransactionStatePurchasing:
                break;
            case SKPaymentTransactionStateRestored:
                ++restoreCount;
            case SKPaymentTransactionStatePurchased:{
                
                NSString *identifier = transaction.payment.productIdentifier;
                
                dispatch_block_t finish = ^{
                    [inProgress removeObject:identifier];
                    
                    dispatch_block_t success = [productToSuccess objectForKey:identifier];
                    if (success != nil) {
                        if ([self isAvailable:identifier]) {
                            success();
                        }
                    }
                    [productToSuccess removeObjectForKey:identifier];
                };
                
                
                if ([[self class] useServer]) {
                    // this is not possible at all on os x
#if TARGET_OS_IPHONE
                    NSData *data = transaction.transactionReceipt;
                    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
                        [self checkReceipt:data forProduct:identifier andThen:^(BOOL succeeded){
                            
                            if (succeeded) {
                                [queue finishTransaction:transaction];
                            }
                            dispatch_async(dispatch_get_main_queue(),finish);
                        }];
                    });
#endif
                } else {
                    // trigger recheck (only on ios7)
                    [self refresh];
                    
                    finish();
                    
                    [queue finishTransaction:transaction];
                }
                    
                break;
            }
            case SKPaymentTransactionStateFailed: {
                NSError *error = transaction.error;
#if TARGET_OS_IPHONE
                generalAlert = [[UIAlertView alloc]
                                      initWithTitle:[error localizedDescription]
                                      message:[error localizedRecoverySuggestion]
                                      delegate:nil
                                      cancelButtonTitle:@"Dismiss"
                                      otherButtonTitles:nil];
                [generalAlert show];
#endif
                
                [queue finishTransaction:transaction];
                [inProgress removeObject:transaction.payment.productIdentifier];
                [productToSuccess removeObjectForKey:transaction.payment.productIdentifier];
                break;
            }
        }
    }
}

- (void)paymentQueue:(SKPaymentQueue *)queue updatedDownloads:(NSArray *)downloads
{
    // we are not interested in downloads
}

- (void)restorePurchases
{
    restoreCount = 0;
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error
{
#if TARGET_OS_IPHONE
    generalAlert = [[UIAlertView alloc]
                    initWithTitle:[error localizedDescription]
                    message:[error localizedRecoverySuggestion]
                    delegate:nil
                    cancelButtonTitle:@"Dismiss"
                    otherButtonTitles:nil];
    [generalAlert show];
#endif
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue
{
#if TARGET_OS_IPHONE
    generalAlert = [[UIAlertView alloc]
                    initWithTitle:@"Success"
                    message:[NSString stringWithFormat:@"Restored %d items", restoreCount]
                    delegate:nil
                    cancelButtonTitle:@"Dismiss"
                    otherButtonTitles:nil];
    [generalAlert show];
#endif
}

@end
