//
//  GearSliderCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 1/26/13.
//
//

#import "GearSliderCell.h"
#import "CocoaThemeManager.h"
#import "GearVolumeSliderCell.h"
#include "IApp.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "Painter.h"
#include "StringUtility.h"
#include "Rules.h"
#import "AlbumSizeSliderCell.h"
#import "AppDelegate.h"

@interface GearSliderCell () {
    BOOL savedContext;
}
@end

@implementation GearSliderCell

using namespace Gear;
using namespace Gui;

- (CGFloat)barWidth
{
    return 3;
}

- (NSImage *)imageForPart:(const std::string &)part
{
    auto rules = IApp::instance()->themeManager()->style().get(convertString(self.style) + " " + part);
    auto name = rules.imageName();
    if (!name.empty()) {
        return [NSImage imageNamed:convertString(name)];
    }
    return nil;
}

- (std::string)system:(const std::string &)specific
{
    if (specific.empty()) {
        return IApp::instance()->themeManager()->style().get(convertString(self.style)).rules().get("osx","").asString();
    } else {
        return IApp::instance()->themeManager()->style().get(convertString(self.style)).rules().get(specific,Json::objectValue).get("osx","").asString();
    }
}

- (void)drawWithFrame:(NSRect)cellFrame
               inView:(NSView *)controlView
{
    // lion draws something extra which we don't need, except when we do...
    if ([self system:""].empty()) {
        CGFloat borderX = yosemiteOrBetter() ? 0 : 4;
        [self drawInteriorWithFrame:CGRectMake(cellFrame.origin.x+borderX, cellFrame.origin.y, cellFrame.size.width-2*borderX, cellFrame.size.height) inView:controlView];
    } else {
        [super drawWithFrame:cellFrame inView:controlView];
    }
}

- (void)drawBarInside:(NSRect)barRect flipped:(BOOL)flipped
{
    barRect.origin.y = self.controlView.bounds.origin.y;
    barRect.size.height = self.controlView.bounds.size.height;
    //NSLog(@"%@ %@ %@ %@", [self class], NSStringFromRect(self.controlView.bounds), NSStringFromRect(self.controlView.superview.bounds), NSStringFromRect([self knobRectFlipped:NO]));
    
    // +: down
    if (yosemiteOrBetter() && self.controlSize != NSRegularControlSize) {
        barRect.size.height -= 1;
    }
    if (!yosemiteOrBetter()) {
        barRect.size.height += 1;
    }
        
    //[[NSColor redColor] setFill]; NSRectFill(self.controlView.bounds);
    //[[NSColor greenColor] setFill]; NSRectFill(barRect);
    
    if (self.style) {
        std::string system = [self system:""];
        if (!system.empty()) {
            [super drawBarInside:barRect flipped:flipped];
            return;
        }
        
        NSImage *left = [self imageForPart:"left"];
        NSImage *leftend = [self imageForPart:"leftend"];
        NSImage *right = [self imageForPart:"right"];
        NSImage *rightend = [self imageForPart:"rightend"];
        
        if (left || right) {
        
            CGFloat rectHeight = barRect.size.height;
            int yOffset = barRect.origin.y + (rectHeight - left.size.height)/2;
            int divider = CGRectGetMidX([self knobRectFlipped:NO]);
            
            [leftend drawInRect:CGRectMake(barRect.origin.x,yOffset,leftend.size.width,leftend.size.height) fromRect:CGRectMake(0,0,leftend.size.width,leftend.size.height) operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
            
            [[NSColor colorWithPatternImage:left] setFill];
            NSRectFill(CGRectMake(barRect.origin.x + leftend.size.width,yOffset,divider-leftend.size.width,left.size.height));

            [[NSColor colorWithPatternImage:right] setFill];
            NSRectFill(CGRectMake(divider,yOffset,barRect.origin.x+barRect.size.width - rightend.size.width-divider,right.size.height));
            
            [rightend drawInRect:CGRectMake(barRect.origin.x+barRect.size.width - rightend.size.width,yOffset,rightend.size.width,rightend.size.height) fromRect:CGRectMake(0,0,rightend.size.width,rightend.size.height) operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
            
            return;
        }
    }
    
    if ([[CocoaThemeManager sharedManager] themePrefix] != nil || ![self isKindOfClass:[GearVolumeSliderCell class]]) {
        [self drawLegacyBarInside:barRect flipped:flipped];
        return;
    }
    
    auto sliderLook = IApp::instance()->themeManager()->current()->volumeSlider();
    auto barWidth = sliderLook.thickness;
    CGFloat upperY = ceil(barRect.origin.y + barRect.size.height /2 - (barWidth / 2));
    //NSLog(@"bar width: %f %@", barWidth, NSStringFromRect(barRect));

    CGRect rect = CGRectMake(barRect.origin.x, upperY, barRect.size.width, barWidth);
    CGRect leftRect = rect;
    CGRect knobRect = [self knobRectFlipped:NO];
    leftRect.origin.x += sliderLook.sideLength;
    leftRect.size.width = knobRect.origin.x + knobRect.size.width/2 - barRect.origin.x;
    
    Painter::paint(*sliderLook.minimumTrack, leftRect);
    
    CGRect rightRect = rect;
    rightRect.origin.x += leftRect.size.width;
    rightRect.size.width -= (leftRect.size.width + sliderLook.sideLength);
    Painter::paint(*sliderLook.maximumTrack, rightRect);
    
    CGRect leftSideRect = CGRectMake(barRect.origin.x, upperY, sliderLook.sideLength, barWidth);
    CGRect rightSideRect = CGRectMake(barRect.origin.x + barRect.size.width - sliderLook.sideLength, upperY, sliderLook.sideLength, barWidth);
    if (sliderLook.leftSide) {
        Painter::paint(*sliderLook.leftSide, leftSideRect);
    }
    if (sliderLook.rightSide) {
        Painter::paint(*sliderLook.rightSide, rightSideRect);
    }
}

- (void)drawLegacyBarInside:(NSRect)barRect flipped:(BOOL)flipped
{
    if ([[CocoaThemeManager sharedManager] themePrefix] != nil && [self isMemberOfClass:[GearSliderCell class]]) {
        // upper right
        barRect.origin.y += 1;
    }
    static const int kAllowedHeight = 20;
    if (barRect.size.height < kAllowedHeight) {
        barRect.origin.y -= (kAllowedHeight - barRect.size.height)/2;
        barRect.size.height = kAllowedHeight;
    }
    
    NSGraphicsContext *context = [NSGraphicsContext currentContext];
    [context saveGraphicsState];
    
    // disable clipping, so there will be no glitches when fast moving knob
    // on the other hand, this caused blipping...
    //[[NSBezierPath bezierPathWithRect:barRect] setClip];
    
    CGFloat barWidth = [self barWidth];
    //NSLog(@"bar width legacy: %f", barWidth);

    
    CGFloat upperY = barRect.origin.y + barRect.size.height / 2 - ((int)barWidth / 2);
    CGFloat midY = barRect.origin.y + barRect.size.height/2;
    
    CGFloat radius = barWidth/2;
    
    CGRect rect = CGRectMake(barRect.origin.x, upperY, barRect.size.width, barWidth);
    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:rect xRadius:radius yRadius:radius];
    NSBezierPath *upperPath = [NSBezierPath bezierPath];
    [upperPath moveToPoint:NSMakePoint(barRect.origin.x, midY)];
    [upperPath appendBezierPathWithArcFromPoint:NSMakePoint(barRect.origin.x, upperY-1) toPoint:NSMakePoint(barRect.origin.x + barRect.size.width/2, upperY-1) radius:radius];
    [upperPath appendBezierPathWithArcFromPoint:NSMakePoint(barRect.origin.x + barRect.size.width, upperY-1) toPoint:NSMakePoint(barRect.origin.x + barRect.size.width, midY) radius:radius];
    
    NSShadow *shadow = [NSShadow new];
    [shadow setShadowColor: [NSColor colorWithDeviceWhite: 0.0f alpha: 1.0f]];
    [shadow setShadowBlurRadius: 0.0f];
    [shadow setShadowOffset: NSMakeSize(0, 1)];
    //[shadow set];
    
    [[[CocoaThemeManager sharedManager] sliderBottom] setStroke];
    [path stroke];
    NSRect leftRect = rect;
    NSRect knobRect = [self knobRectFlipped:NO];
    leftRect.size.width = knobRect.origin.x + knobRect.size.width/2 - barRect.origin.x;
    path = [NSBezierPath bezierPathWithRoundedRect:leftRect xRadius:radius yRadius:radius];
    [[[CocoaThemeManager sharedManager] sliderLeft] setFill];
    [path fill];
    
    NSRect rightRect = rect;
    rightRect.origin.x += leftRect.size.width;
    rightRect.size.width -= leftRect.size.width;
    path = [NSBezierPath bezierPathWithRoundedRect:rightRect xRadius:radius yRadius:radius];
    [[[CocoaThemeManager sharedManager] sliderRight] setFill];
    [path fill];
    [[[CocoaThemeManager sharedManager] sliderTop] setStroke];
    [upperPath stroke];
    
    [context restoreGraphicsState];
}

- (NSImage *)knobImage
{
    if (self.style) {
        NSImage *image = [self imageForPart:"knob"];
        if (image) {
            return image;
        }
    }
    
    NSGradient *gradient = [[CocoaThemeManager sharedManager] ovalButtonGradient:NO];
    if (gradient == nil) {
        return [NSImage imageNamed:@"knob-elliptic-cocoa"];
    } else {
        if ([self isKindOfClass:[GearVolumeSliderCell class]]) {
            return [NSImage imageNamed:@"knob-big"];
        }
        return [NSImage imageNamed:@"knob-elliptic-moderndark"];
    }
}



- (void)drawKnob:(NSRect)knobRect
{
    if (!yosemiteOrBetter() && self.controlSize != NSRegularControlSize) {
        knobRect.size.height += 1;
    }
    if ([[CocoaThemeManager sharedManager] themePrefix] != nil && [self isMemberOfClass:[GearSliderCell class]]) {
        knobRect.origin.y += 1;
    }
    
    std::string system = [self system:""];
    if (system.empty()) {
        system = [self system:"knob"];
    }
    if (!system.empty()) {
        if (system == "mini") {
            [self setControlSize:NSMiniControlSize];
        } else if (system == "small") {
            [self setControlSize:NSSmallControlSize];
        } else if (system == "regular") {
            [self setControlSize:NSRegularControlSize];
        }
        [super drawKnob:knobRect];
        return;
    }
    
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
    
    if ([self imageForPart:"knob"] == nil) {
        if (diameter <= 0.001f && [[self class] isEqualTo:[GearVolumeSliderCell class]]) {
            drawRect.origin.y += 2;
        } else {
            drawRect.origin.y += [self knobOffset];
        }
    }
    [image drawInRect:drawRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
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
        if ([self isKindOfClass:[GearVolumeSliderCell class]]) {
            return 22;
        }
        return 0;
        //return 12;
    }
}


@end
