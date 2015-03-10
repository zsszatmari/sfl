//
//  UITableViewCell+FixUITableViewCellAutolayout.m
//  G-Ear iOS
//
//  Created by zsszatmari on 11/01/15.
//  Copyright (c) 2015 Treasure Box. All rights reserved.
//

#import <objc/runtime.h>
#import <objc/message.h>
#import "UITableViewCell+FixUITableViewCellAutolayout.h"
#import "AppDelegate.h"

@implementation UITableViewCell (FixUITableViewCellAutolayout)

+ (void)load
{
    if ([AppDelegate runningPreIOS7]) {
        Method existing = class_getInstanceMethod(self, @selector(layoutSubviews));
        Method new = class_getInstanceMethod(self, @selector(_autolayout_replacementLayoutSubviews));
        
        method_exchangeImplementations(existing, new);
    }
}

- (void)_autolayout_replacementLayoutSubviews
{
    [super layoutSubviews];
    [self _autolayout_replacementLayoutSubviews]; // not recursive due to method swizzling
    [super layoutSubviews];
}

@end
