//
//  AppleKeychain.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#include "AppleKeychain.h"
#include "KeychainHandler.h"
#include "StringUtility.h"

@interface KeychainHandlerForService : KeychainHandler {
    NSString *service;
}

- (id)initWithService:(NSString *)service;

@end

@implementation KeychainHandlerForService

- (id)initWithService:(NSString *)aService
{
    self = [super init];
    if (self) {
        service = aService;
    }
    return self;
}

- (NSString *)serviceName
{
    return service;
}

@end


namespace Gear
{
#define method AppleKeychain::
    
    string method username(const string &identifier) const
    {
        @autoreleasepool {
            return convertString([[[KeychainHandlerForService alloc] initWithService:convertString(identifier)] keyChainUserName]);
        }
    }
    
    string method pass(const string &identifier) const
    {
        @autoreleasepool {
            return convertString([[[KeychainHandlerForService alloc] initWithService:convertString(identifier)] keychainData]);
        }
    }
    
    void method save(const string &identifier, const string &username, const string &data)
    {
        @autoreleasepool {
            [[[KeychainHandlerForService alloc] initWithService:convertString(identifier)] saveKeychainData:convertString(data) forName:convertString(username)];
        }
    }

    void method forget(const string &identifier)
    {
        @autoreleasepool {
            [[[KeychainHandlerForService alloc] initWithService:convertString(identifier)] forgetKeychainData];
        }
    }
};