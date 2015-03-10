//
//  AnimationUtility.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 21/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AnimationUtility : NSObject

+ (void)springAnimation:(dispatch_block_t)animations completion:(void(^)(BOOL))completion;
+ (void)ios6Animation:(dispatch_block_t)animations completion:(void(^)(BOOL))completion;

+ (NSTimeInterval)animationDuration;

@end
