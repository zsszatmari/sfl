//
//  GearTopBar.h
//  G-Ear iOS
//
//  Created by Gál László on 9/23/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//
#import "GearImageView.h"

@interface GearTopBar : GearImageView<UISearchBarDelegate>

@property(nonatomic, assign) CGFloat leftMargin;
@property(nonatomic, assign) CGFloat topMargin;
@property(nonatomic, assign) CGFloat rightMargin;
@property(nonatomic, assign) CGFloat bottomMargin;

- (void)hideSearchBar;
+ (CGFloat)statusBarHeight;

- (void)settingsShown;
- (void)settingsHidden;
- (void)willRotate;
- (void)applyTheme;

@end
