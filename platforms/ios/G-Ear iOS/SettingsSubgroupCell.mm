//
//  SettingsSubgroupCell.m
//  G-Ear iOS
//
//  Created by Gál László on 8/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SettingsSubgroupCell.h"
#include "StringUtility.h"

using namespace Gear;
@implementation SettingsSubgroupCell

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}


- (void)setPreference:(Gear::Preference &)preference
{
    self.textLabel.text = convertString(preference.title());
    
}

@end
