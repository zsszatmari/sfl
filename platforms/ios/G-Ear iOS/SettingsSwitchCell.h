//
//  SettingsSwitchCell.h
//  G-Ear iOS
//
//  Created by Gál László on 8/15/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SettingsCell.h"
@interface SettingsSwitchCell : SettingsCell
@property (weak, nonatomic) IBOutlet UILabel *textLabel;
@property (weak, nonatomic) IBOutlet UISwitch *valueSwitch;

@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *activityIndicator;
- (void) setPreference:(Gear::Preference &)preference;
- (IBAction)switchValueChanged:(id)sender;

@end
