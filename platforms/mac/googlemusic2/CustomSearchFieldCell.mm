//
//  CustomSearchFieldCell.m
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 10/11/14.
//
//

#import "CustomSearchFieldCell.h"
#import "CocoaThemeManager.h"
#import "AppDelegate.h"

@implementation CustomSearchFieldCell


- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSGradient *searchBackground = [[CocoaThemeManager sharedManager] searchBackground];
    if (searchBackground == nil) {
        [super drawWithFrame:cellFrame inView:controlView];
        return;
    }
    
    CGFloat radius = cellFrame.size.height / 2;
    // avoid problems with borders being thinner than pixel
    cellFrame.origin.x += 0.5f;
    cellFrame.size.width -= 1.0f;
    cellFrame.origin.y += 0.5f;
    cellFrame.size.height -= 1.0f;
    
    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:cellFrame xRadius:radius yRadius:radius];
    [searchBackground drawInBezierPath:path angle:90.0f];
    
    // the interior does not like partial pixels
    CGRect interior = cellFrame;
    interior.origin.x -= 0.5f;
    interior.size.width += 1.0f;
    interior.origin.y += 0.5f;
    interior.size.height -= 1.0f;
    
    [super drawInteriorWithFrame:interior inView:controlView];
    
    [[NSColor colorWithDeviceWhite:24.0f/255.0f alpha:1.0f] setStroke];
    [path setLineWidth:1.5f];
    [path stroke];
}


- (void)drawFocusRingMaskWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    if ([[CocoaThemeManager sharedManager] themePrefix] == nil && lionOrBetter()) {
        [super drawFocusRingMaskWithFrame:cellFrame inView:controlView];
    } else {
        // nada
    }
}

@end
