//
//  GearTabBarItem.h
//  G-Ear iOS
//
//  Created by Gál László on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "stdplus.h"
#import "GearImageView.h"

@interface GearTabBarItem : UITabBarItem
@property(nonatomic, assign) shared_ptr<Gui::IPaintable> icon;
@end
