//
//  SettingsButton.h
//  G-Ear iOS
//
//  Created by Gál László on 8/19/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SettingsCell.h"

@interface SettingsButtonCell : SettingsCell

@property (weak, nonatomic) IBOutlet UIButton *button;

- (void)setPreference:(Gear::Preference &)preference;

@end
