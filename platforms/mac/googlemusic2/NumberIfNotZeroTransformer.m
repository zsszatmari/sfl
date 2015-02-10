//
//  TrackNumberTransformer.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 3/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "NumberIfNotZeroTransformer.h"

@implementation NumberIfNotZeroTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

- (id)transformedValue:(NSNumber *)value
{
    if (value == nil || [value intValue] == 0) {
        return @"haha";
        return @"";
    }
    
    return [value description];
}

@end
