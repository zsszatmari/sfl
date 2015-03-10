//
//  SettingsButton.m
//  G-Ear iOS
//
//  Created by Gál László on 8/19/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SettingsButtonCell.h"
#include "StringUtility.h"

using namespace Gear;

@implementation SettingsButtonCell {
    Preference _preference;
}

- (void)setPreference:(Gear::Preference &)preference
{
    self.button.titleLabel.font = [UIFont systemFontOfSize:15.0f];
    [self.button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    
    _preference = preference;
    
    [self.button setTitle:convertString(preference.title()) forState:UIControlStateNormal];
    self.button.selected = preference.value() ? YES: NO;
    
    [self.button addTarget:self action:@selector(pushed:) forControlEvents:UIControlEventTouchUpInside];
}

- (IBAction)pushed:(id)sender
{
    // should be:
    _preference.setValue(true);
}

@end
