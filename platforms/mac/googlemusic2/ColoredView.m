//
//  ColoredView.m
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 09/11/14.
//
//

#import "ColoredView.h"

@implementation ColoredView {
    NSColor *backgroundColor;
}

- (void)drawRect:(NSRect)dirtyRect
{
    CGFloat alpha = [backgroundColor alphaComponent];
    if (alpha > 0) {
        [backgroundColor setFill];
        NSRectFill(dirtyRect);
    }
}

- (void)setBackgroundColor:(NSColor *)color
{
    backgroundColor = color;
}

@end
