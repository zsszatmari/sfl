//
//  SongsViewController.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/7/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GearTopBar.h"
#import "SlidingView.h"
#import "ElasticTabbar.h"
#include "ValidPtr.h"
#include "Chain.h"

typedef enum
{
    TRANSITION_FROM_LEFT = 1,
    TRANSITION_FROM_RIGHT = -1,
    TRANSITION_NONE = 0
} TransitionDirection;

extern int SETTINGS_TAG;

namespace Gear
{
    class SongEntry;
}

@class FastScrollIndex;

////UITabBarDelegate
@interface SongsViewController : UIViewController <UICollectionViewDelegate, UINavigationControllerDelegate,
    UIViewControllerAnimatedTransitioning,
    UIViewControllerTransitioningDelegate,
    collection_didselect_delegate>
@property (weak, nonatomic) IBOutlet SlidingView *slidingView;

@property (weak, nonatomic) IBOutlet UIView *contentView;
////@property (weak, nonatomic) IBOutlet GearTabBar *tabBar;
@property (weak, nonatomic) IBOutlet GearTopBar *topBar;

@property (strong, nonatomic) ElasticTabbar *tabBar;

@property (nonatomic) TransitionDirection transition_dir;

// an UINavigationController makes no sense here, complicates things unnecessarily, which leads to subtle bugs...
@property (nonatomic, strong) UIViewController *songListNaviController;

+(UINavigationController*) topNavigationController;
+ (SongsViewController *)sharedController;
- (BOOL)settingsActive;
- (void)showPlayer;
- (void)applyTheme;

// performing simultanous transitions cause serious problems (e.g. frozen UINavigationController) so at least they must be wrapped
- (void)performTransition:(dispatch_block_t)block;

@end
