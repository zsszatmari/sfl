//
//  RectButtonCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 3/27/13.
//
//

#import "RectButtonCell.h"
#import "CocoaThemeManager.h"
#import "NSImage+Tinting.h"
#import "AppDelegate.h"

@implementation RectButtonCell
 

- (void)awakeFromNib
{
}

- (void)setTitle:(NSString *)aString
{
    [super setTitle:aString];
}

- (BOOL)isDown
{
    BOOL down;
    if ([self showsStateBy] == 0) {
        down = [self isHighlighted];
    } else {
        down = [self state] != 0;
    }
    return down;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
//    cellFrame.origin.y -= 0;
//    cellFrame.size.height += 2;
    [super drawWithFrame:cellFrame inView:controlView];
}

- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    NSString *prefix = nil;
    // this was wrong: makes buttons ugly when not main window
    //if ([controlView.window isMainWindow]) {
        prefix = [[CocoaThemeManager sharedManager] themePrefix];
    //}
    
    // systemSupplied: that is, the share button. we don't draw a bezel for the share button, not even in modern dark theme
    if (prefix == nil || self.systemSupplied) {
        frame.origin.y +=1;
        if ([self image] == nil) {
            [super drawBezelWithFrame:frame inView:controlView];
        }
        return;
    }
    BOOL down = [self isDown];
    
    NSImage *image;
    if (down) {
        image = [NSImage imageNamed:[prefix stringByAppendingString:@"-rectbutton-selected"]];
    } else {
        image = [NSImage imageNamed:[prefix stringByAppendingString:@"-rectbutton"]];
    }

 
    const CGFloat kSide = 14.0f;
    NSRect leftFrame = NSMakeRect(frame.origin.x, frame.origin.y, kSide, frame.size.height);
    NSRect rightFrame = NSMakeRect(frame.origin.x + frame.size.width - kSide, frame.origin.y, kSide, frame.size.height);
    NSRect middleFrame = NSMakeRect(frame.origin.x + kSide, frame.origin.y, frame.size.width - 2 * kSide, frame.size.height);
    
    //CGFloat scale = [[controlView window] backingScaleFactor];
    NSRect leftSource = NSMakeRect(0, 0, kSide * 2, image.size.height);
    NSRect middleSource = NSMakeRect(kSide, 0, image.size.width - 2 * kSide *2, image.size.height);
    NSRect rightSource = NSMakeRect(image.size.width - kSide *2, 0, kSide*2, image.size.height);
    
    if (self.systemSupplied) {
        
        // cut the bottom part...
        static const int kCut = 1;
        
        leftSource.origin.y += kCut;
        middleSource.origin.y += kCut;
        rightSource.origin.y += kCut;
        
        leftSource.size.height -= kCut;
        middleSource.size.height -= kCut;
        rightSource.size.height -=kCut;
        
        leftFrame.size.height -= kCut;
        middleFrame.size.height -= kCut;
        rightFrame.size.height -= kCut;
    }
    
    [image drawInRect:leftFrame fromRect:leftSource operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
    [image drawInRect:middleFrame fromRect:middleSource operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
    [image drawInRect:rightFrame fromRect:rightSource operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
    
    if (self.systemSupplied) {
        [[NSColor colorWithDeviceWhite:56.0f/255.0f alpha:1.0f] setFill];
        
        NSRectFill(CGRectMake(0, 0, 1, 1));
        NSRectFill(CGRectMake(frame.size.width-1, 0, 1, 1));
    }
}

- (NSColor *)pressedColor
{
      //  return [NSColor colorWithDeviceRed:63.0f/255.0f green:101.0f/255.0f blue:178.0f/255.0f alpha:1.0f];
    return [NSColor colorWithDeviceRed:0.0f/255.0f green:103.0f/255.0f blue:210.0f/255.0f alpha:1.0f];
}

- (void)drawImage:(NSImage *)image withFrame:(NSRect)frame inView:(NSView *)controlView
{
    //NSLog(@"drawimage image: %@ alternate: %@ %@ %d %d %@ %d '%@'", [image name], [[self alternateImage] name], NSStringFromRect(frame), self.bezelStyle, self.controlSize, NSStringFromSize(self.cellSize), self.imagePosition, self.stringValue);
    NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
    if (prefix == nil) {
        frame.origin.y += 1;
    }
    
    if (prefix == nil && [self isDown]) {
        image = [self alternateImage];
    }
    if (self.systemSupplied) {
        if (prefix == nil && [self isDown]) {
            
            NSImage *pressedImage;
           
            NSString *name = [image name];
            pressedImage = [NSImage imageNamed:[name stringByAppendingString:@"-pressed"]];
            [pressedImage imageTintedWithColor:[NSColor whiteColor]];
            if (pressedImage) {
                image = pressedImage;
            }
        } else {
            if (prefix == nil) {
            } else {
                image = [image imageTintedWithColor:[NSColor whiteColor]];
            }
        }
    }
    //NSLog(@"drawimage rect: %@ image: %@", NSStringFromRect(frame), NSStringFromSize([image size]));
    
  /*  NSString *name = [image name];
    if (!lionOrBetter() && [name isEqualToString:@"outputs"]) {
        [super drawImage:image withFrame:frame inView:controlView];
        return;
    }
    
    frame.origin.y -= 1;
    
    CGSize newSize;
    if (self.systemSupplied) {
        newSize = image.size;
    } else {
        if (image.size.height < 20) {
            newSize = image.size;
        } else {
            newSize = CGSizeMake(image.size.width /2, image.size.height /2);
        }
    }
    frame.origin.x += (frame.size.width - newSize.width)/2;
    frame.origin.y += (frame.size.height - newSize.height)/2;
    frame.size = newSize; 
    
    NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
    if (prefix == nil) {
        frame.origin.y += 1;
    }
    
    */
    [super drawImage:image withFrame:frame inView:controlView];
}

- (NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)frame inView:(NSView *)controlView
{
    frame.origin.y -= 1;
    
    NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
    
    NSColor *color = [[CocoaThemeManager sharedManager] buttonContentColor];
    if (prefix == nil && [self isDown]) {
        
        //color = [self pressedColor];
    }
    
    NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:[title string]];
    NSRange range = NSMakeRange(0, [string length]);
    [string addAttribute:NSForegroundColorAttributeName value:color range:range];
    NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
    paragraphStyle.alignment = NSCenterTextAlignment;
    [string addAttribute:NSParagraphStyleAttributeName value:paragraphStyle range:range];
    //[string addAttribute:NSFontAttributeName value:[NSFont fontWithName:@"HelveticaNeue" size:11.0f] range:range];
    if (prefix == nil) {
        [string addAttribute:NSFontAttributeName value:[NSFont systemFontOfSize:11.0f] range:range];
    } else {
        [string addAttribute:NSFontAttributeName value:[NSFont fontWithName:@"HelveticaNeue-Medium" size:11.0f] range:range];
    }
    
    //[string addAttribute:NSBaselineOffsetAttributeName value:@(1) range:range];

    frame.origin.y -= 1;
    if (prefix == nil) {
        frame.origin.y += 1;
    } else {
        frame.origin.y -= 2;
    }
    frame.size.height += 3;
    
    CGContextRef ctx = (CGContextRef)[NSGraphicsContext currentContext].graphicsPort;
    CGContextSetShouldSmoothFonts(ctx, false);
    
    [string drawInRect:frame];
    return CGRectZero;
    
    //return [super drawTitle:string withFrame:frame inView:controlView];
}

@end
