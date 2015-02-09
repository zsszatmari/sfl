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
    NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
    if (prefix == nil) {
        [self drawBezelWithFrameNoPrefix:frame inView:controlView];
        return;
    }
        
    NSImage *image;
    if ([self down]) {
        image = [NSImage imageNamed:[prefix stringByAppendingString:@"-roundbutton-selected"]];
    } else {
        image = [NSImage imageNamed:[prefix stringByAppendingString:@"-roundbutton"]];
    }
    
    [image drawInRect:frame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
}

- (void)drawBezelWithFrameNoPrefix:(NSRect)frame inView:(NSView *)controlView
{
    const CGFloat kMargin = 3.0f;
    frame = CGRectMake(frame.origin.x + kMargin, frame.origin.y + kMargin, frame.size.width - 2 *kMargin, frame.size.height - 2 * kMargin);
    
    // leave place for shadow
    NSBezierPath *path = [NSBezierPath bezierPathWithOvalInRect:CGRectMake(frame.origin.x+1, frame.origin.y +1, frame.size.width -2, frame.size.height-2)];
    [path setLineWidth:0.25f];
    
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:[self state] == NSOnState];
    if (gradient != nil) {
        
        NSShadow *shadow = [[NSShadow alloc] init];
        [shadow setShadowColor:[NSColor colorWithDeviceWhite:91.0f/255.0f alpha:1.0f]];
        [shadow setShadowOffset:NSMakeSize(0, -1)];
        [shadow set];
        
        [gradient drawInBezierPath:path angle:270];
        [[NSColor colorWithDeviceWhite:15.0f/255.0f alpha:1.0f] setStroke];
        [path stroke];
        
        if (![self isEnabled]) {
            return;
        }
        if ([self down]) {
            
            [gradient drawInBezierPath:path angle:90];
            // photoshop: 13px, 63%, 92/140/232 blue
            NSGradient *innerGlow = [[NSGradient alloc] initWithColorsAndLocations:
                                     [NSColor clearColor], 0.0f,
                                     [NSColor clearColor], 0.01f,
                                     [NSColor colorWithDeviceRed:92.0f/255.0f green:92.0f/255.0f blue:92.0f/255.0f alpha:0.3f], 1.0f, nil];
            [innerGlow drawInBezierPath:path relativeCenterPosition:NSMakePoint(0, 0.0)];
            
        }
    }
}

- (void)drawImage:(NSImage *)image withFrame:(NSRect)frame inView:(NSView *)controlView
{
    //NSLog(@"image size: %@ frame: %@ view: %@", NSStringFromSize([image size]), NSStringFromRect(frame), NSStringFromRect([controlView frame]));
    
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:[self state] == NSOnState];
    
    float kMinify = 1.0f;
    
    frame = NSMakeRect((int)(frame.origin.x + frame.size.width * (1-kMinify)/2) + self.xOffset, (int)(frame.origin.y + frame.size.height * (1-kMinify) / 2), (int)(frame.size.width * kMinify), (int)(frame.size.height * kMinify));
    
    if (gradient) {
        //frame.origin.y += 0.5f;
        // very hacky, I know...
        /*if (image.size.width == 31) {
            frame.origin.x -= 1;
        } else if (image.size.width == 12) {
            // pause
            frame.size.width += 2;
            
            if ([CorrectedButtonCell isRetina:controlView]) {
            //    frame.origin.x += 1;
                frame.origin.y += 1;
            }
            
        } else if (image.size.width == 19) {
            // play button
            frame.origin.x += 1;
            
            if ([CorrectedButtonCell isRetina:controlView]) {
                frame.origin.x += 2;
            }
        }*/
    } else {
        if (image.size.width == 17) {
            // play button
            frame.origin.x += 2;
        } else if (image.size.width == 29) {
            // next
            frame.origin.x += 1;
        }
    }
    
    if (image.size.width == 19) {
        // play button
        //frame.origin.y -= 1;
    }
    
    
    [image drawInRect:frame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
    // drawInRect is mavericks only
    //[image drawInRect:frame];
    
    //[[NSColor blackColor] setFill];
    //NSRectFillUsingOperation(frame, NSCompositeSourceAtop);
}

@end
