//
//  AlignedTextField.m
//  Gear for Google Play and YouTube
//
//  Created by Zsolt Szatmari on 10/02/15.
//
//

#import "AlignedTextField.h"
#import "NSView+PixelGrid.h"

@implementation AlignedTextField

- (void)drawRect:(NSRect)dirtyRect
{
    [self alignToPixelGrid];
    
    [super drawRect:dirtyRect];
}

@end
