//
//  EqSlider.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 14/02/15.
//  Copyright (c) 2015 Treasure Box. All rights reserved.
//

#import "EqSlider.h"
#include "IApp.h"
#include "IPreferences.h"
#include "StringUtility.h"
#include "IEqualizer.h"

using namespace Gear;

@implementation EqSlider {
    UILabel *label;
    UISlider *slider;
}

- (id)init
{
    self = [super init];
    if (self) {
        //self.backgroundColor = [[UIColor greenColor] colorWithAlphaComponent:0.3f];
        
        slider = [[UISlider alloc] init];
        slider.translatesAutoresizingMaskIntoConstraints = YES;
        slider.transform = CGAffineTransformMakeRotation(-M_PI_2);
        [self addSubview:slider];
        
        slider.minimumValue = -12;
        slider.maximumValue = +12;
        
        
        slider.frame = CGRectMake(0, 0, self.frame.size.width, self.frame.size.height);
        slider.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    }
    return self;
}

- (void)setLabelText:(std::string)labelText
{
    if (label == nil) {
        
        label = [[UILabel alloc] init];
        label.translatesAutoresizingMaskIntoConstraints = NO;
        
        [self.superview addSubview:label];
        label.font = [UIFont systemFontOfSize:10];
        label.textAlignment = NSTextAlignmentCenter;
        
        // the label is below us
        [self.superview addConstraint:[NSLayoutConstraint constraintWithItem:label attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterX multiplier:1 constant:0]];
        [self.superview addConstraint:[NSLayoutConstraint constraintWithItem:label attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1 constant:0]];
    }
    label.text = convertString(labelText);
}

- (void)update:(BOOL)animated
{
    float value = IApp::instance()->preferences().floatForKey(self.key);

    [slider setValue:value animated:animated];
}

- (CGSize)intrinsicContentSize
{
    return CGSizeMake(30, UIViewNoIntrinsicMetric);
}

- (void)userInput:(CGPoint)point
{
    float value = 12-24*(point.y / self.frame.size.height);
    [slider setValue:value animated:NO];
    IApp::instance()->preferences().setFloatForKey(self.key, value);
    IApp::instance()->preferences().setUintForKey("eqEnabled", true);
    
    IApp::instance()->equalizer()->notifyChange();
    self.onUpdate();
}

@end
