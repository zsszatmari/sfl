//
//  AlbumSizeSliderCell.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 22/06/14.
//
//

#import "AlbumSizeSliderCell.h"
#import "CocoaThemeManager.h"

@implementation AlbumSizeSliderCell

- (NSImage *)knobImage
{
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:NO];
    if (gradient == nil) {
        return [NSImage imageNamed:@"knob-cocoa"];
    } else {
        return [NSImage imageNamed:@"knob-moderndark"];
    }
}

- (int)knobOffset
{
    return 0;
}

@end
