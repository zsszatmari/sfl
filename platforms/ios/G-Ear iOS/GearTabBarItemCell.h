//
//  GearTabBarItemView.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 09/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GearTabBarItem.h"
#import "ElasticTabBar.h"


@interface GearTabBarItemCell : UIView

@property(nonatomic, assign) Gui::Color unselected_color;
@property(nonatomic, assign) Gui::Color selected_color;
@property(nonatomic, weak) ElasticTabbar *tabBar;

- (void)setTabBarItem:(GearTabBarItem *)item;
- (void)setSelectedTag:(int)tag;

@end
