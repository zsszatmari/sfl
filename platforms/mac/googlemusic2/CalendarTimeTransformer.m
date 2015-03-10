//
//  CalendarTimeTransformer.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/8/12.
//
//

#import "CalendarTimeTransformer.h"

@implementation CalendarTimeTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

- (id)transformedValue:(NSNumber *)value
{
    long long nanoSecs = [value longLongValue];
    return [self transformedNanoSecs:nanoSecs];
}
    
- (id)transformedNanoSecs:(long long)nanoSecs
{
    if (nanoSecs == 0) {
        return @"";
    }
    NSDate *date = [NSDate dateWithTimeIntervalSince1970:nanoSecs/1000000];
    
    static dispatch_once_t onceToken;
    static NSDateFormatter *formatter;
    dispatch_once(&onceToken, ^{
        formatter = [[NSDateFormatter alloc] init];
        [formatter setDateStyle:NSDateFormatterShortStyle];
        [formatter setTimeStyle:NSDateFormatterShortStyle];
    });

    return [formatter stringFromDate:date];
}

@end
