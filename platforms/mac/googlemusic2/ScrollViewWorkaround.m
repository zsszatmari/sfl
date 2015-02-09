//
//  ScrollViewWorkaround.m
//  G-Ear
//
//  Created by Zsolt Szatmari on 6/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "ScrollViewWorkaround.h"

@implementation ScrollViewWorkaround

NSString * const kNotificationScrollByUser = @"NotificationScrollByUser";

- (void)setScrollerStyle:(NSScrollerStyle)newScrollerStyle
{
    if ([super respondsToSelector:@selector(setScrollerStyle:)]) {
        [super setScrollerStyle:[NSScroller preferredScrollerStyle]];
    }
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    [super scrollWheel:theEvent];
    [[NSNotificationCenter defaultCenter] postNotificationName:kNotificationScrollByUser object:self];
}

@end
