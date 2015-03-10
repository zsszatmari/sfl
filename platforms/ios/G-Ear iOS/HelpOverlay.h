//
//  HelpOverlay.h
//  NewsBar mobile
//
//  Created by Zsolt Szatmári on 4/16/13.
//  Copyright (c) 2013 Merlin Developments. All rights reserved.
//

#import <UIKit/UIKit.h>

extern NSString * const kDefaultsPrefixOverlay;

@interface HelpOverlay : UIImageView

+ (void)showIfNecessary:(NSString *)identifier inView:(UIView *)view offset:(CGFloat)offset;
+ (void)showIfNecessary:(NSString *)identifier fromController:(UIViewController *)controller;
+ (void)showIfNecessary:(NSString *)identifier fromController:(UIViewController *)controller pieces:(NSDictionary *)pieces;
+ (void)showForced:(NSString *)identifier;
+ (void)markIdentifier:(NSString *)identifier;

@end
