//
//  NSImage+Tinting.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 4/6/13.
//
//

#import <QuartzCore/QuartzCore.h>
#import "NSImage+Tinting.h"

@implementation NSImage (Tinting)

- (NSImage *)imageTintedWithColor:(NSColor *)aColor
{
    NSImage *image = self;
    if (aColor == nil) {
        return image;
    }
    
    NSBitmapImageRep *rep = [[image representations] objectAtIndex: 0];
    NSSize imageSize = NSMakeSize([rep pixelsWide], [rep pixelsHigh]);
    if (imageSize.height == 0) {
        imageSize = [image size];
        NSImage *newImage = [[NSImage alloc] initWithSize:imageSize];
        [newImage lockFocus];
        
        NSRect imageRect = {NSZeroPoint, imageSize};
        [image drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0f];
        [newImage unlockFocus];
        image = newImage;
    }
    
    NSSize virtualSize = [image size];
    [image setSize:virtualSize];
    
    NSRect imageRect = {NSZeroPoint, /*imageSize*/virtualSize};
    
    NSImage *newImage = [[NSImage alloc] initWithSize:image.size];
    [newImage lockFocus];
    [image drawInRect:imageRect fromRect:imageRect operation:NSCompositeCopy fraction:1.0f];
    [aColor set];
    NSRectFillUsingOperation(imageRect, NSCompositeSourceAtop);
    [newImage unlockFocus];
    return newImage;
}

- (NSImage *)imageTintedWithColorKeepSize:(NSColor *)aColor
{
    NSImage *image = self;
    if (aColor == nil) {
        return image;
    }
    
    NSSize imageSize = [image size];
    NSRect imageRect = {NSZeroPoint, imageSize};
    
    [image lockFocus];
    [aColor set];
    NSRectFillUsingOperation(imageRect, NSCompositeSourceAtop);
    [image unlockFocus];
    return image;
}

+ (NSImage *)imageNamed:(NSString *)name tintWithColor:(NSColor *)aColor
{
    NSImage *image = [[NSImage imageNamed:name] copy];
 
    return [image imageTintedWithColor:aColor];
}

@end
