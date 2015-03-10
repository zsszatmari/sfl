//
//  GearSlider.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 8/17/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <vector>
#import "GearSlider.h"
#include "Painter.h"
#include "Color.h"
#include "Gradient.h"
#include "IApp.h"
#include "ThemeManager.h"
#include "ITheme.h"

using namespace Gui;
using namespace Gear;

@implementation GearSlider {
    UIImage *leftSide;
    float thickness;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
    }
    return self;
}

- (BOOL)beginTrackingWithTouch:(UITouch *)touch withEvent:(UIEvent *)event
{
    BOOL ret = [super beginTrackingWithTouch:touch withEvent:event];
    if (ret) {
        return YES;
    }
    
    // just a tap
    float newValue = ((float)[touch locationInView:self].x) / self.frame.size.width;
    
    [super setValue:newValue animated:YES];
    [self sendActionsForControlEvents:UIControlEventValueChanged];
    return NO;
}

- (BOOL)continueTrackingWithTouch:(UITouch *)touch withEvent:(UIEvent *)event
{
    self.disableTime = [NSDate timeIntervalSinceReferenceDate] + 1.5f;
    return [super continueTrackingWithTouch:touch withEvent:event];
}
    
- (void)setValue:(float)value animated:(BOOL)animated
{
    if (value >= 0.999f) {
        // problem on right side
        value = 0.999f;
    }
    [super setValue:value animated:animated];
}

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        self.continuous = NO;
        [self applyTheme];
    }
    return self;
} 

 

// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    [super drawRect:rect];
    
    //self.backgroundColor = [UIColor redColor];
    
    [leftSide drawInRect:CGRectMake(1, self.frame.size.height/2-(3.0f/2), 1, thickness)];
    // Drawing code
}

- (void)applyTheme
{
    auto sliderLook = IApp::instance()->themeManager()->current()->seekSlider();
    thickness = sliderLook.thickness;
    
    {
        UIImage *side = Painter::convertImage(sliderLook.thumb, CGSizeMake(sliderLook.thumbWidth, thickness));
        [self setThumbImage:side forState:UIControlStateNormal];
        [self setThumbImage:side forState:UIControlStateHighlighted];
        
        [self setThumbImage:side forState:UIControlStateDisabled];
        
        leftSide = Painter::convertImage(sliderLook.leftSide, CGSizeMake(1.0f, thickness));
    }
    
    {
        [self setMinimumTrackImage:Painter::convertImage(sliderLook.minimumTrack, CGSizeMake(1, thickness)) forState:UIControlStateNormal];
    }
    
    {
        if (YES || UIDeviceOrientationIsPortrait([UIDevice currentDevice].orientation)) {
            [self setMaximumTrackImage:Painter::convertImage(sliderLook.maximumTrack, CGSizeMake(1, thickness)) forState:UIControlStateNormal];
        } else {
            shared_ptr<Gui::IPaintable> color(new Color(51.0f/255.0f,53.0f/255.0f,52.0f/255.0f,1.0f));
            [self setMaximumTrackImage:Painter::convertImage(color, CGSizeMake(1, 3.5f)) forState:UIControlStateNormal];
        }
    }

    [self setNeedsDisplay];
}

@end
