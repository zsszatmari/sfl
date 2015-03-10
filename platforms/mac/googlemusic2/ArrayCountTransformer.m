//
//  ArrayCountTransformer.m
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/11/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "ArrayCountTransformer.h"

@implementation ArrayCountTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

- (id)transformedValue:(NSArray *)value
{
    NSUInteger count = [value count];
    return [self transformedInt:count];
}
    
- (NSString *)transformedInt:(NSUInteger)count
{
    if (count > 1) {
        return [NSString stringWithFormat:@"%ld songs",count];
    } else {
        return [NSString stringWithFormat:@"%ld song",count];
    }
}

@end
