//
//  GearTabBarItem.m
//  G-Ear iOS
//
//  Created by Gál László on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "GearTabBarItem.h"
#include MEMORY_H

@implementation GearTabBarItem

- (BOOL) isEqual: (id) object {
	return ([self.title isEqual: [object title]] && self.tag == [object tag]);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%d %@",[self tag], [self title]];
}

@end
