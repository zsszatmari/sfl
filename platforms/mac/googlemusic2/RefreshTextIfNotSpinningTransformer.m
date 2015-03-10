//
//  RefreshTextIfNotSpinningTransformer.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 3/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "RefreshTextIfNotSpinningTransformer.h"

@implementation RefreshTextIfNotSpinningTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

- (id)transformedValue:(NSNumber *)value
{
    return ![value boolValue] ? @"" : @"Refresh";
}

@end
