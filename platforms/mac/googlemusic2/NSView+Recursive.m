//
//  NSView+Recursive.m
//  Gear for Google Play and YouTube
//
//  Created by Zsolt Szatmari on 14/01/15.
//
//

#import "NSView+Recursive.h"

@implementation NSView (Recursive)

- (void)setNeedsDisplayRecursive
{
    [self setNeedsDisplay:YES];
    for (NSView *view in [self subviews]) {
        [view setNeedsDisplayRecursive];
    }
}

@end
