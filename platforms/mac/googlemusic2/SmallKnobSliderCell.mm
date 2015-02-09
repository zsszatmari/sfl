//
//  SmallKnobSliderCell.m
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/24/13.
//
//

#import "SmallKnobSliderCell.h"
#import "CocoaThemeManager.h"
#import "GearVolumeSliderCell.h"
#import "AlbumSizeSliderCell.h"

@implementation SmallKnobSliderCell

- (void)drawKnob:(NSRect)knobRect
{
    CGFloat diameter = [self knobSize];
    
    NSImage *image = [self knobImage];
    CGFloat width;
    CGFloat height;
    if (diameter <= 0.001f || [self isKindOfClass:[AlbumSizeSliderCell class]]) {
        width = [image size].width;
        height = [image size].height;
    } else {
        width = diameter;
        height = diameter;
    }
    
    NSRect drawRect = NSMakeRect(knobRect.origin.x + (int)((knobRect.size.width - width)/2), knobRect.origin.y + (int)((knobRect.size.height - height)/2) , width, height);
    if (diameter <= 0.001f && [[self class] isEqualTo:[GearVolumeSliderCell class]]) {
        drawRect.origin.y += 2;
    } else {
        drawRect.origin.y += [self knobOffset];
    }
    [image drawInRect:drawRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
}
 
- (NSImage *)knobImage
{
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:NO];
    if (gradient == nil) {
        return [NSImage imageNamed:@"knob-elliptic-cocoa"];
    } else {
        return [NSImage imageNamed:@"knob-elliptic-moderndark"];
    }
}

- (int)knobOffset
{
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:NO];
    if (gradient == nil) {
        return 0;
    } else {
        return 0;
    }
}

- (CGFloat)knobSize
{
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:NO];
    if (gradient == nil) {
        return 0;
    } else {
        return 0;
        //return 12;
    }
}

@end
