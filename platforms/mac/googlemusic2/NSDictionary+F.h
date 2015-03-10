//
//  NSDictionary+F.h
//  Gear for Google Play and YouTube
//
//  Created by Zsolt Szatmari on 13/01/15.
//
//

#import <Foundation/Foundation.h>

@interface NSDictionary (F)

- (NSDictionary *)addObject:(id)object forKey:(id<NSCopying>)key;

@end
