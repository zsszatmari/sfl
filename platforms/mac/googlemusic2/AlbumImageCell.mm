//
//  AlbumImageCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/3/13.
//
//

#import "AlbumImageCell.h"
#import "CocoaThemeManager.h"
#import "AppDelegate.h"

@implementation AlbumImageCell

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    if (yosemiteOrBetter() && [[CocoaThemeManager sharedManager] themePrefix] == nil) {
        // draw separator line instead
        [super drawWithFrame:cellFrame inView:controlView];
        return;
    }
    NSGradient *gradient = [[CocoaThemeManager sharedManager] albumArtBorderGradient];
    
    const CGFloat kExternalWidth = 1.0f;
    CGFloat kOuterWidth;
    if ([[CocoaThemeManager sharedManager] themePrefix] == nil) {
        kOuterWidth = 1.0f;
    } else {
        kOuterWidth = 2.0f;
    }
    NSRect rect;
    
    NSColor *colorTop;
    NSColor *colorBottom;
    [gradient getColor:&colorTop location:NULL atIndex:[gradient numberOfColorStops]-1];
    [gradient getColor:&colorBottom location:NULL atIndex:0];
    // bottom
    rect = NSMakeRect(cellFrame.origin.x + kExternalWidth, cellFrame.origin.y + kExternalWidth, cellFrame.size.width - 2 *kExternalWidth, kOuterWidth);
    [colorBottom setFill];
    NSRectFill(rect);
    
    // top
    rect.origin.y = cellFrame.size.height - kOuterWidth - kExternalWidth;
    [colorTop setFill];
    NSRectFill(rect);
    
    // left
    rect = NSMakeRect(cellFrame.origin.x + kExternalWidth, cellFrame.origin.y + kExternalWidth, kOuterWidth, cellFrame.size.height - 2 * kExternalWidth);
    [gradient drawInRect:rect angle:90.0f];
    
    // right
    rect.origin.x = cellFrame.size.width - kOuterWidth - kExternalWidth;
    [gradient drawInRect:rect angle:90.0f];
    
    const CGFloat kBorderWidth = kOuterWidth + 2.0f;
    
    NSRect innerFrame = NSMakeRect(cellFrame.origin.x + kBorderWidth, cellFrame.origin.y + kBorderWidth, cellFrame.size.width - 2*kBorderWidth, cellFrame.size.height - 2 *kBorderWidth);
    [super drawWithFrame:innerFrame inView:controlView];
}

@end
