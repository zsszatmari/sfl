//
//  KeychainHandler.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 11/2/12.
//
//

#import <Foundation/Foundation.h>

@interface KeychainHandler : NSObject {
    NSString *name;
}

- (void)saveKeychainData:(NSString *)data forName:(NSString *)aName;
- (void)forgetKeychainData;
- (NSString *)keyChainUserName;
- (NSString *)keychainData;

@end
