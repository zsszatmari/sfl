//
//  GearSlider.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/28/12.
//
//

// the purpose of this class is to ignore value updates when seeking

#import <Cocoa/Cocoa.h>

@interface GearSlider : NSSlider

- (void)setDoubleValueForced:(double)aDouble;

@end
