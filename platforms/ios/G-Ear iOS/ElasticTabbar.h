//
//  ElasticTabbar.h
//  G-Ear iOS
//
//  Created by Szabo Attila on 12/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GearImageView.h"
#include "stdplus.h"

@protocol collection_didselect_delegate <NSObject>
- (void)tabBarDidSelectTag:(NSInteger) index_path;
@end

@class GearTabBarItem;

@interface ElasticTabbar : UIView<UIGestureRecognizerDelegate> 
}

@property(nonatomic, assign) shared_ptr <Gui::IPaintable> background;
@property(nonatomic, assign) Gui::Color unselected_color;
@property(nonatomic, assign) Gui::Color selected_color;
@property(nonatomic, strong) UIScrollView *collection_view;

@property (nonatomic) int selected_tag;

@property (nonatomic, weak) NSObject <collection_didselect_delegate> *didselect_delegate;

- (void)select_item_by_tag: (int) tag;
- (void)set_items: (NSArray *) items;
- (void)applyTheme;

- (void)didRotateInterface;

@end
