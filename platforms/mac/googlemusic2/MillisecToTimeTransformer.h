//
//  MillisecToTimeTransformer.h
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MillisecToTimeTransformer : NSValueTransformer

+ (NSString *)stringFromInterval:(NSTimeInterval)interval;
- (id)transformedInt:(int)value;

@end
