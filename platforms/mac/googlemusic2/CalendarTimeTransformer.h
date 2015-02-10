//
//  CalendarTimeTransformer.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/8/12.
//
//

#import <Cocoa/Cocoa.h>

@interface CalendarTimeTransformer : NSValueTransformer

- (id)transformedNanoSecs:(long long)value;

@end
