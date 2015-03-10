//
//  SettingsTextCell.m
//  G-Ear iOS
//
//  Created by Gál László on 8/15/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SettingsTextCell.h"
#include "StringUtility.h"

using namespace Gear;

@implementation SettingsTextCell {
    UILabel *state;
    NSTimer *timer;
    Preference _preference;
}

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
    }
    return self;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

- (void)setPreference:(Gear::Preference &)preference
{
    _preference = preference;
    self.textLabel.text = convertString(preference.title());
    self.valueLabel.text = [NSString stringWithFormat:@"%lld", preference.value()];

    if (preference.type() == Preference::Type::Special || preference.type() == Preference::Type::Options) {
        if (state == nil) {
            state = [[UILabel alloc] initWithFrame:CGRectMake(self.frame.size.width-160, (self.frame.size.height-20)/2, 120, 20)];
            state.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
            state.textAlignment = NSTextAlignmentRight;
            [state setEnabled:NO];
            [state setBackgroundColor:[UIColor clearColor]];
            [self addSubview:state];
        }
        [state setHidden:NO];
        [self setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
        [self setSelectionStyle:UITableViewCellSelectionStyleBlue];
        
        timer = [NSTimer timerWithTimeInterval:0.4f target:self selector:@selector(updateState) userInfo:nil repeats:YES];
        [[NSRunLoop mainRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
        [self updateState];
    } else {
        [state setHidden:YES];
        [self setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
        [self setSelectionStyle:UITableViewCellSelectionStyleNone];
    }
}

- (void)updateState
{
    if (_preference.type() == Preference::Type::Special) {
        if (_preference.value()) {
            [state setText:@"On"];
        } else {
            [state setText:@"Off"];
        }
    } else if (_preference.type() == Preference::Type::Options) {
        [state setText:convertString(_preference.options().at(_preference.value()))];
    }
}

@end
