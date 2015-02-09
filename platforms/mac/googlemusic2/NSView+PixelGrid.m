//
//  NSView+PixelGrid.m
//  Gear for Google Play and YouTube
//
//  Created by Zsolt Szatmari on 05/12/14.
//
//

#import "NSView+PixelGrid.h"

@implementation NSView (PixelGrid)

- (void)alignToPixelGrid
{
    CGRect frame = self.frame;
    if ((frame.origin.y - floor(frame.origin.y)) > 0.1) {
        frame.origin.y = floor(frame.origin.y);
        self.frame = frame;
    }
    if ((frame.origin.x - floor(frame.origin.x)) > 0.1) {
        frame.origin.x = floor(frame.origin.x);
        self.frame = frame;
    }
}

@end
