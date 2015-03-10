//
//  NSObject+Ext.m
//  G-Ear iOS
//
//  Created by Szabo Attila on 11/3/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "NSObject+Ext.h"

@implementation NSObject(Ext)

- (NSString *) class_name {
	return NSStringFromClass(self.class);
}

@end
