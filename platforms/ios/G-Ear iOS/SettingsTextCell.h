//
//  SettingsTextCell.h
//  G-Ear iOS
//
//  Created by Gál László on 8/15/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SettingsCell.h"
@interface SettingsTextCell : SettingsCell
@property (weak, nonatomic) IBOutlet UILabel *textLabel;
@property (weak, nonatomic) IBOutlet UILabel *valueLabel;
//- (void) setPreference:(Gear::Preference &)preference;
@end
