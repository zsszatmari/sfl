//
//  ClickableImageView.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/7/12.
//
//

#import "ClickableImageView.h"

@implementation ClickableImageView

- (void)mouseDown:(NSEvent *)event {
    
    if ([[self target] respondsToSelector:[self action]]) {
        [NSApp sendAction:[self action] to:[self target] from:self];
    }
}

- (NSSize)intrinsicContentSize
{
    // don't let autolayout modify the superview's sizes
    return NSMakeSize(NSViewNoInstrinsicMetric, NSViewNoInstrinsicMetric);
}

@end
