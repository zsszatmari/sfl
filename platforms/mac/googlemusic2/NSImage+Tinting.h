//
//  NSImage+Tinting.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 4/6/13.
//
//

#import <Cocoa/Cocoa.h>

@interface NSImage (Tinting)

+ (NSImage *)imageNamed:(NSString *)name tintWithColor:(NSColor *)color;
- (NSImage *)imageTintedWithColor:(NSColor *)aColor;
- (NSImage *)imageTintedWithColorKeepSize:(NSColor *)aColor;


@end
