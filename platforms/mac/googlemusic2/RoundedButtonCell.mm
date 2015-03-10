//
//  RoundedButtonCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 3/10/13.
//
//

#import "RoundedButtonCell.h"
#import "CocoaThemeManager.h"
#import "CorrectedButtonCell.h"

@implementation RoundedButtonCell

- (BOOL)down
{
    BOOL down;
    if ([self showsStateBy] == 0) {
        down = [self isHighlighted];
    } else {
        down = [self state] != 0;
    }
    return down;
}

- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    NSImage *image;
    if ([self down]) {
        image = self.bezelAlternateImage;
    } else {
        image = self.bezelImage;
    }
    if (image != nil) {
        CGRect rect = CGRectMake((int)(frame.origin.x + (frame.size.width - image.size.width)/2),
                            (int)(frame.origin.y + (frame.size.height - image.size.height)/2),
                            (int)image.size.width,
                            (int)image.size.height);
        [image drawInRect:rect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
        return;
    }
}

- (void)drawImage:(NSImage *)image withFrame:(NSRect)frame inView:(NSView *)controlView
{
    // no disabled graying
    
    [image drawInRect:frame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
    // drawInRect is mavericks only
    //[image drawInRect:frame];
}

@end
