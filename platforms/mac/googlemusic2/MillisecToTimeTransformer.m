//
//  MillisecToTimeTransformer.m
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "MillisecToTimeTransformer.h"

@implementation MillisecToTimeTransformer

+ (NSString *)stringFromInterval:(NSTimeInterval)interval
{
    return [NSString stringWithFormat:@"%02d:%02d", ((int)interval)/ 60, ((int)interval) % 60];
}

+ (Class)transformedValueClass {
    return [NSString class];
}

- (id)transformedValue:(NSNumber *)value
{
    if (value == nil) {
        return @"";
    }

    return [self transformedInt:[value intValue]];
}

- (id)transformedInt:(int)value
{
    if (value == 0) {
        return @"";
    }
    NSTimeInterval interval = value / 1000.0f;
    return [MillisecToTimeTransformer stringFromInterval:interval];
}

@end
