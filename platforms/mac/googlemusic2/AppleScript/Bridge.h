//
//  Bridge.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 01/04/14.
//
//

#import <Foundation/Foundation.h>

@interface Bridge : NSObject

+ (Bridge *)sharedBridge;
- (NSString *)registerObject:(id)object;
- (id)objectFor:(NSString *)identifier;

@end
