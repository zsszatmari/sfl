//
//  AppDelegate.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 6/23/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>

@class LockScreenController;
@interface AppDelegate : UIResponder <UIApplicationDelegate> {
	LockScreenController *lockInfo;
}

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) UIViewController *topViewController;

- (UIStoryboard *)storyBoard;
- (void)sendFeedback;
+ (AppDelegate *)sharedDelegate;
+ (BOOL) runningPreIOS8;
+ (BOOL) runningPreIOS7;
+ (BOOL) runningPreIOS6;
+ (BOOL) runningPreI5;
+ (UIImageView *) snapshotFromView: (UIView *) view;

@end
