//
//  RatingToImageTransformer.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/10/12.
//
//

#import "RatingToImageTransformer.h"

@implementation RatingToImageTransformer

+ (Class)transformedValueClass {
    return [NSImage class];
}

- (id)transformedValue:(NSNumber *)value
{
    return [NSImage imageNamed:@"thumbsup"];
}


@end
