//
//  EQTickView.m
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/24/13.
//
//

#import "EQTickView.h"

@implementation EQTickView

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor colorWithDeviceWhite:0.6 alpha:1.0f] setFill];
    NSRectFill(dirtyRect);
}

@end
