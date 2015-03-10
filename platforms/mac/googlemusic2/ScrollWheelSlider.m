//
//  ScrollWheelSlider.m
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/28/13.
//
//

#import "ScrollWheelSlider.h"

@implementation ScrollWheelSlider

- (void)scrollWheel:(NSEvent*) event
{
    float range = [self maxValue] - [self minValue];
    float increment = (range * [event deltaY]) / 100;
    BOOL inverted = NO;
    if ([event respondsToSelector:@selector(isDirectionInvertedFromDevice)] && [event isDirectionInvertedFromDevice]) {
        inverted = YES;
    }
    if (inverted) {
        increment *= -1;
    }
    float val = [self floatValue] + increment;
    
    BOOL wrapValue = ([[self cell] sliderType] == NSCircularSlider);
    
    if (wrapValue) {
        if ( val < [self minValue])
            val = [self maxValue] - fabs(increment);
        
        if( val > [self maxValue])
            val = [self minValue] + fabs(increment);
    }
    
    [self setFloatValue:val];
    [self sendAction:[self action] to:[self target]];
}

@end
