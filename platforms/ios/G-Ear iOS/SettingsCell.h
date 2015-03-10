//
//  SettingsCell.h
//  G-Ear iOS
//
//  Created by Gál László on 8/15/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Preference.h"

@interface SettingsCell : UITableViewCell
- (void) setPreference:(Gear::Preference &)preference;

@end
