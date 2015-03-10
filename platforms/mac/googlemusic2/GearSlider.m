//
//  GearSlider.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/28/12.
//
//

#import <QuartzCore/QuartzCore.h>
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

+ (id)defaultAnimationForKey:(NSString *)key
{
    if ([key isEqualToString:@"doubleValue"]) {
        return [CABasicAnimation animation];
    } else {
        return [super defaultAnimationForKey:key];
    }
}

@end

@implementation AnimationForSlider {
    
    NSSlider *slider;
    float animateToValue;
    double max;
    double min;
    float initValue;
}
@synthesize slider;
@synthesize animateToValue;


-(void)startAnimation
{
    //Setup initial values for every animation
    initValue = [slider floatValue];
    if (animateToValue >= initValue) {
        min = initValue;
        max = animateToValue;
    } else  {
        min = animateToValue;
        max = initValue;
    }
    
    [super startAnimation];
}


- (void)setCurrentProgress:(NSAnimationProgress)progress
{
    [super setCurrentProgress:progress];
    
    double newValue;
    if (animateToValue >= initValue) {
        newValue = min + (max - min) * progress;
    } else  {
        newValue = max - (max - min) * progress;
    }
    
    [slider setDoubleValue:newValue];
}

@end
