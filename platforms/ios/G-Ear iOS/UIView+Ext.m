//
//  UIView+Ext.m
//  G-Ear iOS
//
//  Created by Szabo Attila on 11/3/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "UIView+Ext.h"

@implementation UIView(Ext)

- (NSArray *) recursive_subviews {
	NSMutableArray *sv = [NSMutableArray arrayWithObject: self];
	for (UIView *v in [self subviews])
		[sv addObjectsFromArray: [v recursive_subviews]];
	return sv;
//	[sv copy];
}

@end
