//
//  VerticallyCenteredTextField.m
//  Gear for Google Play
//
//  Created by Zsolt Szatmari on 18/11/14.
//
//

#import <Quartz/Quartz.h>
#import "VerticallyCenteredTextField.h"
#import "CocoaThemeManager.h"
#import "NSView+PixelGrid.h"

@interface VerticallyCenteredTextFieldCell : NSTextFieldCell

@property (nonatomic,assign) BOOL disableAntialias;
@property (nonatomic,assign) BOOL enableAntialias;
@property (nonatomic,assign) int verticalAlignment;

@end

@implementation VerticallyCenteredTextFieldCell

@synthesize verticalAlignment;

- (NSRect)titleRectForBounds:(NSRect)theRect
{
    NSRect titleFrame = [super titleRectForBounds:theRect];
    //CGSize cellSize = self.cellSize;
    //CGRect titleFrame = CGRectMake(0, 0, cellSize.width, cellSize.height);
    
    NSSize titleSize = [[self attributedStringValue] size];
    if (verticalAlignment == 0) {
        titleFrame.origin.y = (int)(theRect.origin.y + (theRect.size.height - titleSize.height) / 2.0);
    } else if (verticalAlignment < 0) {
        titleFrame.origin.y = theRect.origin.y + (theRect.size.height - titleSize.height);
    } else if (verticalAlignment > 0) {
        titleFrame.origin.y = theRect.origin.y;
        if ([[CocoaThemeManager sharedManager] themePrefix] != nil) {
            // this is incorrectly calculated...
            titleFrame.origin.y -= 2;
        }
        
    }
    return titleFrame;
}

//extern "C" void CGContextSetFontSmoothingBackgroundColor(CGContextRef, CGColorRef);

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSRect titleRect = [self titleRectForBounds:cellFrame];
    /*if (verticalAlignment > 0) {
        [[[NSColor redColor] colorWithAlphaComponent:0.5f] setFill];
        NSRectFill(titleRect);
    } else if (verticalAlignment < 0) {
        [[[NSColor greenColor] colorWithAlphaComponent:0.5f] setFill];
        NSRectFill(titleRect);
    }*/
    
    BOOL applyAntialias = [[CocoaThemeManager sharedManager] themePrefix] != nil;
    applyAntialias = NO;
    
    
    //[[NSColor redColor] setFill]; NSRectFill(cellFrame);
    //[[NSColor blueColor] setFill]; NSRectFill(titleRect);
    
    uint32_t background;
    uint32_t *data;
    CGContextRef bitmapContext;
    CGContextRef ctx = (CGContextRef)[NSGraphicsContext currentContext].graphicsPort;
    
    if ((self.disableAntialias || [[CocoaThemeManager sharedManager] themePrefix] != nil) && !self.enableAntialias) {
        CGContextSetShouldSmoothFonts(ctx, false);
        //CGContextSetAllowsFontSmoothing(ctx, false);
        //CGContextSetShouldSubpixelPositionFonts(ctx, false);
        //CGContextSetAllowsFontSubpixelPositioning(ctx, false);
        
        if ([[self stringValue] isEqualToString:@"Amduscia"]) {
            NSView *s = self.controlView;
            while (s) {
                NSLog(@"HAHAh %@ %@", NSStringFromRect([s.superview convertRect:[s frame] toView:nil]), s);
                s = [s superview];
            }
        }
    }
    if (applyAntialias) {
        [[NSGraphicsContext currentContext] setShouldAntialias:YES];
        
        CGContextSetShouldSmoothFonts(ctx, true);
        // this would be cool but it's private api:
        //CGContextSetFontSmoothingBackgroundColor(ctx, [NSColor whiteColor].CGColor);
        
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        bitmapContext = CGBitmapContextCreate(NULL, cellFrame.size.width, cellFrame.size.height, 8, cellFrame.size.width * 4, colorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
        CGColorSpaceRelease(colorSpace);
        
        [NSGraphicsContext saveGraphicsState];
        NSGraphicsContext* newCtx = [NSGraphicsContext
                                     graphicsContextWithGraphicsPort:bitmapContext flipped:true];
        [NSGraphicsContext setCurrentContext:newCtx];

        CGContextSetShouldSmoothFonts(bitmapContext, true);
        [[NSGraphicsContext currentContext] setShouldAntialias:YES];
        
        //CGContextSetFontSmoothingBackgroundColor(bitmapContext, [NSColor whiteColor].CGColor);
        
        if ([[CocoaThemeManager sharedManager] themePrefix] == nil) {
            background = 0xFFDCDCDC;
        } else {
            background = 0xFF333333;
        }
        
        data = (uint32_t *)CGBitmapContextGetData(bitmapContext);
        for (int i = 0 ; i < cellFrame.size.width*cellFrame.size.height ; ++i) {
            //data[i] = background;
            //data[i] = 0xFF000000;
        }
    }
    
    // drawInRect is no good, see e.g. 'Aghora'
    if (self.alignment == NSLeftTextAlignment || self.alignment == NSNaturalTextAlignment || self.alignment == NSJustifiedTextAlignment || self.alignment == NSCenterTextAlignment) {
        
        //[[self stringValue] drawAtPoint:titleRect.origin withAttributes:@{NSForegroundColorAttributeName:[NSColor labelColor]}];
        
        // add ellipsis (...)
        NSAttributedString *str = [self attributedStringValue];
        if ([str size].width > cellFrame.size.width) {
            NSMutableAttributedString *iterate = [str mutableCopy];
            while ([iterate length] > 0) {
                [iterate replaceCharactersInRange:NSMakeRange([iterate length]-1, 1) withString:@""];
                NSMutableAttributedString *withEllipsis = [iterate mutableCopy];
                [withEllipsis replaceCharactersInRange:NSMakeRange([withEllipsis length],0) withString:@"\u2026"];
                if (iterate.size.width < cellFrame.size.width -10) {
                    str = withEllipsis;
                    break;
                }
            }
        }
    
        //[[self stringValue] drawAtPoint:titleRect.origin withAttributes:@{NSFontAttributeName:[NSFont fontWithName:@"HelveticaNeue-Medium" size:12.0f]}];
        
        [controlView alignToPixelGrid];
        /*if (verticalAlignment > 0) {
            
            [controlView alignToPixelGrid];
            NSLog(@"point: %@ %@", NSStringFromRect(titleRect), NSStringFromRect(controlView.frame));
        }*/
        [str drawAtPoint:titleRect.origin];
    } else {
        // right or center alignmnent is more complicated
        [[self attributedStringValue] drawInRect:titleRect];
    }
    
    // setting the background color to white was necessary for proper antialiasing
    // now we remove the whiteness
    
    //uint8_t background_r = (background >> 16) & 0xFF;
    //uint8_t background_g = (background >> 8) & 0xFF;
    //uint8_t background_b = (background) & 0xFF;
    
    if (applyAntialias) {
        for (int i = 0 ; i < cellFrame.size.width*cellFrame.size.height ; ++i) {
            if (data[i] == background) {
                data[i] = 0x00000000;
            }
        }
        
        [NSGraphicsContext restoreGraphicsState];
        CGImageRef image = CGBitmapContextCreateImage(bitmapContext);
        CGContextSetBlendMode(ctx, kCGBlendModeNormal);
        CGContextDrawImage(ctx, cellFrame, image);
        
        CGImageRelease(image);
        CGContextRelease(bitmapContext);
    }
}

@end

@implementation VerticallyCenteredTextField

- (VerticallyCenteredTextField *)alignTop
{
    VerticallyCenteredTextFieldCell *cell = [self cell];
    cell.verticalAlignment = +1;
    return self;
}

- (VerticallyCenteredTextField *)alignBottom
{
    VerticallyCenteredTextFieldCell *cell = [self cell];
    cell.verticalAlignment = -1;
    return self;
}

+ (Class)cellClass
{
    return [VerticallyCenteredTextFieldCell class];
}

- (void)setDisableAntialias:(BOOL)disableAntialias
{
    VerticallyCenteredTextFieldCell *cell = [self cell];
    cell.disableAntialias = disableAntialias;
}

- (void)setEnableAntialias:(BOOL)value
{
    VerticallyCenteredTextFieldCell *cell = [self cell];
    cell.enableAntialias = value;
}

- (NSSize)intrinsicContentSize
{
    NSSize ret = [super intrinsicContentSize];
    ret.width = [self attributedStringValue].size.width;
    //NSLog(@"%@ intrinsic: %@ %@", [self stringValue], NSStringFromSize(ret), NSStringFromSize([[self attributedStringValue] size]));
    return ret;
}

@end
