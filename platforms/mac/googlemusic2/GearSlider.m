//
//  GearSlider.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/28/12.
//
//

#import "GearSlider.h"

@interface GearSlider () {
    BOOL mouseDown;
}
@end

@implementation GearSlider

// this hack is necessary for an accepatble behaviour (not jumping back momentarily!)

- (void)slidingDone
{
    mouseDown = NO;
}

- (void)setDoubleValue:(double)aDouble
{
    if (!mouseDown) {
        [super setDoubleValue:aDouble];
    }
}

- (void)setDoubleValueForced:(double)aDouble
{
    [super setDoubleValue:aDouble];
}

- (BOOL)sendAction:(SEL)theAction to:(id)theTarget
{
    mouseDown = YES;
    
    // runloop magic http://www.cocoabuilder.com/archive/cocoa/170471-nsslider-did-finish-sliding.html
    SEL sel = @selector(slidingDone);
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:sel object:nil];
    [self performSelector:sel withObject:nil afterDelay:0.0];
    
    return [super sendAction:theAction to:theTarget];
}

@end
