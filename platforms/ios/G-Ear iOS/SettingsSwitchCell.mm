//
//  SettingsSwitchCell.m
//  G-Ear iOS
//
//  Created by Gál László on 8/15/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SettingsSwitchCell.h"
#include "StringUtility.h"

using namespace Gear;
@implementation SettingsSwitchCell {
    NSTimer *timer;
    Preference _preference;
}

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
    }
    return self;
}


- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];
 }


- (void)setPreference:(Gear::Preference &)preference
{
    _preference = preference;
    self.textLabel.text = convertString(preference.title());
    [self updateSwitch];
    self.activityIndicator.activityIndicatorViewStyle = UIActivityIndicatorViewStyleGray;
    self.activityIndicator.color = [UIColor blackColor];
    self.activityIndicator.hidesWhenStopped = YES;
    timer = [NSTimer timerWithTimeInterval:0.4f target:self selector:@selector(updateSwitch) userInfo:nil repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
}

- (IBAction)switchValueChanged:(id)sender {

    _preference.setValue(static_cast<int64_t>(self.valueSwitch.on ? Preference::OnOffState::On : Preference::OnOffState::Off));
}

- (void)willMoveToSuperview:(UIView *)newSuperview
{
    [super willMoveToSuperview:newSuperview];
    if(newSuperview == nil && timer != nil) {
        [timer invalidate];
    } else {
        [timer fire];
    }
}

- (void)updateSwitch {
    
    UIColor *textColor = [UIColor blackColor];
    self.valueSwitch.enabled = YES;
    BOOL animating = NO;
    switch(_preference.valueOnOff()) {
        case Preference::OnOffState::Off:
            self.valueSwitch.on = NO;
            break;
        case Preference::OnOffState::On:
            self.valueSwitch.on = YES;
            break;
        case Preference::OnOffState::OnSpinning:
            self.valueSwitch.on = YES;
            animating = YES;
            break;
        case Preference::OnOffState::Inactive:
            self.valueSwitch.on = NO;
            textColor = [UIColor grayColor];
            break;
        case Preference::OnOffState::Disabled:
            self.valueSwitch.on = NO;
            textColor = [UIColor grayColor];
            self.valueSwitch.enabled = NO;
            break;
    }
    
    self.textLabel.textColor = textColor;
    if (animating) {
        [self.activityIndicator startAnimating];
    } else {
        [self.activityIndicator stopAnimating];
    }
}

@end
