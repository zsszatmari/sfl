//
//  LinkButton.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 2/9/13.
//
//

#import "LinkButton.h"

@implementation LinkButton

- (void)resetCursorRects
{
    NSRect bounds = [self bounds];
    [self addCursorRect:bounds cursor:[NSCursor pointingHandCursor]];
}

@end
