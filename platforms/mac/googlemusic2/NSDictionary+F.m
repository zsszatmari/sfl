//
//  NSDictionary+F.m
//  Gear for Google Play and YouTube
//
//  Created by Zsolt Szatmari on 13/01/15.
//
//

#import "NSDictionary+F.h"

@implementation NSDictionary (F)

- (NSDictionary *)addObject:(id)object forKey:(id<NSCopying>)key
{
    NSMutableDictionary *ret = [self mutableCopy];
    [ret setObject:object forKey:key];
    return ret;
}

@end
