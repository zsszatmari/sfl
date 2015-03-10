//
//  GearTabBarItemView.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 09/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import "GearTabBarItemCell.h"
#import "GearImageView.h"
#include "TextAttributes.h"
#include "Writer.h"

@implementation GearTabBarItemCell {
    UILabel *l;
    GearImageView *icon;
    GearTabBarItem *item;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self populate];
    }
    return self;
}

- (void)populate
{
    self.backgroundColor = [UIColor clearColor];
	if (self.subviews.count) {
		[self.subviews makeObjectsPerformSelector: @selector(removeFromSuperview)];
    }
	l = [[UILabel alloc] initWithFrame: CGRectMake((self.frame.size.width - 70)/2, 26+3, 70, 20)];
    //	l.text = _items[index_path.item];
	l.backgroundColor = [UIColor clearColor];
    l.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
	l.font = [UIFont systemFontOfSize: 8];
	l.textAlignment = NSTextAlignmentCenter;
    
	icon = [[GearImageView alloc] initWithFrame: CGRectMake((self.frame.size.width - 24)/2, 6+3, 24, 24)];
    icon.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
    icon.stretchImage = NO;
    
    //cell.backgroundColor = [UIColor greenColor];
    
	[self addSubview: l];
	[self addSubview: icon];
}

- (void)setTabBarItem:(GearTabBarItem *)tabbar_item
{
    item = tabbar_item;
    
    l.text = tabbar_item.title;
    icon.image = tabbar_item.icon;
}

- (void)setSelectedTag:(int)tag
{
    BOOL selected = (item.tag == tag);
    //NSLog(@"TAAAAG %@ %d (%d == %d)", item.title, selected, item.tag, tag);
    
	Gui::Color color = selected ? _selected_color : _unselected_color;
    
    Gui::TextAttributes attributes = Gui::TextAttributes();
    attributes.setColor(color);
    
    Gui::Writer::apply(attributes, l);
    [icon setTintColor: color];
}

@end
