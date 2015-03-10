//
//  AnimationUtility.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 21/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "AnimationUtility.h"
#import "AppDelegate.h"

@implementation AnimationUtility

+ (void)springAnimation:(dispatch_block_t)animations completion:(void(^)(BOOL))completion
{
    if ([AppDelegate runningPreIOS7]) {
        [self ios6Animation:animations completion:completion];
    } else {
        [UIView animateWithDuration:[self animationDuration] delay: .0 usingSpringWithDamping: .53 initialSpringVelocity: 1. options: UIViewAnimationOptionTransitionNone
                     animations: animations                     completion: completion];
    }
}

+ (void)ios6Animation:(dispatch_block_t)animations completion:(void(^)(BOOL))completion
{
    [UIView animateWithDuration: .3 delay: .0 options: UIViewAnimationOptionTransitionNone
                     animations: animations                     completion: completion];
}

+ (NSTimeInterval)animationDuration
{
    return 0.7f;
}

@end
