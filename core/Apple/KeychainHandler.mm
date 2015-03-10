//
//  KeychainHandler.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 11/2/12.
//
//

#include <Security/Security.h>
#import "KeychainHandler.h"
#include "CoreDebug.h"
#include "Environment.h"

@implementation KeychainHandler

- (NSString *)serviceName
{
    NSAssert(NO, @"abstract method servicename");
    return nil;
}

- (const char *)serviceNameC
{
    return [[self serviceName] cStringUsingEncoding:NSUTF8StringEncoding];
}

- (NSDictionary *)query
{
    return @{(__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
             (__bridge id)kSecReturnData: (id)kCFBooleanTrue,
             (__bridge id)kSecReturnAttributes: (id)kCFBooleanTrue,
             (__bridge id)kSecAttrService:[self serviceName],
             (__bridge id)kSecMatchLimit: (__bridge id)kSecMatchLimitOne};
}

- (NSString *)keychainData
{
#if !TARGET_OS_IPHONE
    // check if we have some credentials (session keys) stored
    UInt32 sessionKeyLength;
    void *sessionKeyData;
    SecKeychainItemRef keyChainItem = 0;
    const char *serviceName = [self serviceNameC];
    OSStatus ret = SecKeychainFindGenericPassword(NULL, (UInt32)strlen(serviceName), serviceName, 0, NULL, &sessionKeyLength, &sessionKeyData, &keyChainItem);
    if (ret != 0) {
        if (keyChainItem != 0) {
            CFRelease(keyChainItem);
        }
        return nil;
    }
    NSString *sessionKey = [[NSString alloc] initWithBytes:sessionKeyData length:sessionKeyLength encoding:NSUTF8StringEncoding];
    SecKeychainAttributeList list;
    SecKeychainAttribute nameAttribute;
    nameAttribute.tag = kSecAccountItemAttr;
    nameAttribute.length = 0;
    nameAttribute.data = NULL;
    list.count = 1;
    list.attr = &nameAttribute;
    SecKeychainItemCopyContent(keyChainItem, NULL, &list, NULL, NULL);
    if (nameAttribute.data != NULL) {
        // success!
        name = [[NSString alloc] initWithBytes:nameAttribute.data length:nameAttribute.length encoding:NSUTF8StringEncoding];
        SecKeychainItemFreeContent(&list, NULL);
    }
    
    CFRelease(keyChainItem);
    SecKeychainItemFreeContent(NULL, sessionKeyData);
    
    if (name != nil && sessionKey != nil) {
        return sessionKey;
    } else {
        return nil;
    }
#else
    
    CFDictionaryRef result = 0;
    OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef)[self query], (CFTypeRef *)&result);
    
    NSString *sessionKey = nil;
    NSString *accName = nil;
    if (status == 0) {
        CFDataRef passData = (CFDataRef)CFDictionaryGetValue(result, CFSTR("v_Data"));
        CFDictionaryRef attributes = result;
        
        if (passData != NULL && attributes != NULL) {
            
            sessionKey = [[NSString alloc] initWithData:(__bridge NSData *)passData encoding:NSUTF8StringEncoding];
            
            accName = (NSString *)CFDictionaryGetValue(attributes, kSecAttrAccount);
        }
        
        CFRelease(result);
    }
    
    if (accName != nil && sessionKey != nil) {
        name = accName;
        return sessionKey;
    }
    return nil;
    
#endif
}

- (NSString *)keyChainUserName
{
    if (name == nil) {
        [self keychainData];
    }
    return name;
}

- (void)forgetKeychainData
{
#if !TARGET_OS_IPHONE
    SecKeychainItemRef keyChainItem = 0;
    const char *serviceName = [self serviceNameC];
    OSStatus ret = SecKeychainFindGenericPassword(NULL, (UInt32)strlen(serviceName), serviceName, 0, NULL, NULL, NULL, &keyChainItem);
    if (ret == 0) {
        
        ret = SecKeychainItemDelete(keyChainItem);
        if (ret != 0) {
            NSLog(@"error deleting %@ key: %d", [self serviceName], ret);
        }
    }
    if (keyChainItem != 0) {
        CFRelease(keyChainItem);
    }

#else
    // other parameters are not comprehendable by delete
    NSDictionary *query = @{(__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
                            (__bridge id)kSecAttrService:[self serviceName]};
    
    SecItemDelete((__bridge CFDictionaryRef) query);
#endif
}

- (void)saveKeychainData:(NSString *)data forName:(NSString *)aName
{
    [self forgetKeychainData];
    
#if !TARGET_OS_IPHONE
    const char *cSession = [data cStringUsingEncoding:NSUTF8StringEncoding];
    const char *cName = [aName cStringUsingEncoding:NSUTF8StringEncoding];
    const char *serviceName = [self serviceNameC];
    OSStatus ret = SecKeychainAddGenericPassword(NULL, (UInt32)strlen(serviceName), serviceName, (UInt32)strlen(cName), cName, (UInt32)strlen(cSession), cSession, NULL);
#else
    
    
    NSDictionary *attributes = @{(__bridge id)kSecClass: (__bridge id)kSecClassGenericPassword,
                                 (__bridge id)kSecAttrService:[self serviceName],
                                 (__bridge id)kSecAttrAccount:aName,
                                 (__bridge id)kSecValueData:[data dataUsingEncoding:NSUTF8StringEncoding]
                                 };
    
    OSStatus ret = SecItemAdd((__bridge CFDictionaryRef) attributes, NULL);
#endif
    if (ret != 0) {
        NSLog(@"error saving %@ key: %ld", [self serviceName], ret);
    }
}


@end
