//
//  SongsViewController.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/7/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SongsViewController.h"
#include "App.h"
#include "ISession.h"
#include "StringUtility.h"
#import "SongListController.h"
#import "PlaylistListController.h"
#import "SettingsViewController.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "IService.h"
#include "ServiceManager.h"
#include "Painter.h"
#include "Writer.h"
#include "Gradient.h"
#include "IPreferencesPanel.h"
#include "PreferenceGroup.h"
#include "Preference.h"
#import "AppDelegate.h"
#include "Debug.h"
#import "AnimationUtility.h"
#import "FastScrollIndex.h"
#import "GearTabBarItem.h"
#include "SessionManager.h"
#import "PlayViewController.h"
#include "IPlaylist.h"

using namespace Gear;
using namespace Gui;
static UINavigationController* _topNavigationController=nil;
int SETTINGS_TAG = 10;

@implementation SongsViewController {
    GearTabBarItem *selectedTabBarItem;
    SignalConnection playlistCategoryConnection;
    SignalConnection playlistChangedConnection;
//    UINavigationController* _songListNaviController;
    UIImageView *fakeView;
    BOOL firstAppearance;
    SignalConnection connectedConnection;
    FastScrollIndex *scrollIndex;
    BOOL keepSettingsVisible;
    BOOL transitioning;
    dispatch_block_t pendingTransition;
    int _categoryLevel;
    NSLayoutConstraint *topBarHeightConstraint;
    PlayViewController *playerController;
    
    NSMutableArray *controllers;
}
@synthesize transition_dir;
@synthesize topBar;
@synthesize slidingView;
@synthesize tabBar;

static SongsViewController *_controller;

+ (SongsViewController *)sharedController
{
    return _controller;
}

- (void)updateTabs
{
    auto sessionManager = App::instance()->sessionManager();
    auto playlistsCategories = sessionManager->categories();
    
    NSMutableArray *items = [NSMutableArray array];
    for (auto &p : *playlistsCategories) {
        NSString *title = convertString(p.title());
        GearTabBarItem *item = [[GearTabBarItem alloc] initWithTitle:title image:nil tag:p.tag()];
        item.icon = p.icon();
        [items addObject:item];
    }
    GearTabBarItem *item = [[GearTabBarItem alloc] initWithTitle:@"Settings" image:nil tag:SETTINGS_TAG];
    item.icon = IApp::instance()->themeManager()->current()->settingsIcon();
    
    [items addObject:item];
    
    
    ////////        self.tabBar.maxVisibleTabs = 4;
    ////////        self.tabBar.padding = 10;
   
#ifndef SPLASH_SCREEN
    ///////        [self.tabBar setItems:items animated:NO];
    [self.tabBar set_items: items];
#endif
}

- (void)viewDidLoad
{
    // rare but disturbing SIGPIPE while debugging
    signal(SIGPIPE, SIG_IGN);
    
#ifdef DEBUG
    NSLog(@"screen size: %@", NSStringFromCGSize([UIScreen mainScreen].bounds.size));
#endif
    
    controllers = [NSMutableArray array];
    
    _controller = self;
    
    firstAppearance = YES;

	self.tabBar = [[ElasticTabbar alloc] initWithFrame: CGRectMake(0, self.view.frame.size.height - 50, self.view.frame.size.width, 50)];
	self.tabBar.didselect_delegate = self;
    

    _topNavigationController = self.navigationController;
    self.navigationController.delegate = self;
    [AppDelegate sharedDelegate].topViewController = self.navigationController;
    
    auto c = [self]{
        SVNSLOG(@"categories changed");

        [self updateTabs];

    };
    c();
    playlistChangedConnection =  IApp::instance()->sessionManager()->playlistsChangeEvent().connect(c);
    

////////    self.tabBar.tabBarDelegate = self;
    _songListNaviController = [[UIViewController alloc] init];
    
    [self addChildViewController:_songListNaviController];
    [self.contentView addSubview:_songListNaviController.view];
    _songListNaviController.view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    // must set color before first selecting the default tab, otherwise the color setting won't affect the default selection
    [self applyTheme];

//////    fakeView = [[UIImageView alloc] init];
//////	[_slidingView insertSubview: fakeView atIndex: 0];

    _songListNaviController.view.frame = self.contentView.bounds;
    [self addChildViewController:_songListNaviController];
    //[_songListNaviController didMoveToParentViewController:self];
    
    BOOL everRan = [[NSUserDefaults standardUserDefaults] boolForKey:@"EverRan"];
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"EverRan"];
    [[NSUserDefaults standardUserDefaults] synchronize];
    if (!everRan || IApp::instance()->serviceManager()->generalState() == IService::State::Offline) {
        
        [self.tabBar select_item_by_tag:SETTINGS_TAG];
        [self switchToSettings];
    }
    
    /*scrollIndex = [[FastScrollIndex alloc] initWithFrame:CGRectMake(self.view.frame.size.width - 20, self.topBar.frame.size.height, 20, self.view.frame.size.height -self.topBar.frame.size.height - self.tabBar.frame.size.height)];
    // invisible by default (see first startup, settings screen)
    [scrollIndex setAlpha:0.0f];
    [self.view addSubview:scrollIndex];*/
    
    UIView *container = slidingView.superview;
    
    NSDictionary *views = NSDictionaryOfVariableBindings(topBar,slidingView, tabBar, container);
    
    [container addSubview: self.tabBar];
    topBar.translatesAutoresizingMaskIntoConstraints = NO;
    slidingView.translatesAutoresizingMaskIntoConstraints = NO;
    tabBar.translatesAutoresizingMaskIntoConstraints = NO;
    [container addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[topBar]|" options:0 metrics:nil views:views]];
    [container addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[slidingView]|" options:0 metrics:nil views:views]];
    [container addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[tabBar]|" options:0 metrics:nil views:views]];
    [container addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[topBar][slidingView][tabBar(==50)]|" options:0 metrics:nil views:views]];

    topBarHeightConstraint = [NSLayoutConstraint constraintWithItem:topBar attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:0 constant:0];
    [self setTopBarHeight];
    [container addConstraint:topBarHeightConstraint];
    
    // this must happen after the other views are in place for proper positioning:
    [self connectToPlaylist];
    
    // needed for ios6:
    self.contentView.frame = self.slidingView.bounds;
    self.contentView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
}

- (void)setTopBarHeight
{
    static const CGFloat kGearTopBarHeight = 35.0f;
    CGFloat topBarHeight = [GearTopBar statusBarHeight] + kGearTopBarHeight;
    [self.topBar willRotate];
    topBarHeightConstraint.constant = topBarHeight;
}

- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
                                         duration:(NSTimeInterval)duration
{
    [self setTopBarHeight];
    
    if (![AppDelegate runningPreIOS7]) {
        [[UIApplication sharedApplication] setStatusBarHidden:UIInterfaceOrientationIsLandscape(interfaceOrientation) withAnimation:UIStatusBarAnimationFade];
    }
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    [super didRotateFromInterfaceOrientation:fromInterfaceOrientation];
    
    [self.tabBar didRotateInterface];
    
    //NSLog(@"sliding frame: %@", NSStringFromCGRect(self.slidingView.frame));
    //NSLog(@"contentview frame: %@", NSStringFromCGRect(self.contentView.frame));
}

- (void)setScrollIndex:(FastScrollIndex *)aScrollIndex
{
    if (aScrollIndex == scrollIndex) {
        return;
    }
    UIView *previousIndex = scrollIndex;
    [UIView animateWithDuration:0.3f animations:^{
        [previousIndex setAlpha:0.0f];
    } completion:^(BOOL finished) {
        //if (previousIndex != self->scrollIndex) {
            [previousIndex removeFromSuperview];
        //}
    }];
    scrollIndex = aScrollIndex;
    //scrollIndex.frame = CGRectMake(self.view.frame.size.width - 20, self.topBar.frame.size.height, 20, self.view.frame.size.height -self.topBar.frame.size.height - self.tabBar.frame.size.height);
    //scrollIndex.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleHeight;
    
    if (scrollIndex) {
        scrollIndex.translatesAutoresizingMaskIntoConstraints = NO;
        [self.view addSubview:scrollIndex];
        NSDictionary *views = NSDictionaryOfVariableBindings(scrollIndex,topBar,tabBar);
        [self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[scrollIndex(==20)]|" options:0 metrics:nil views:views]];
        [self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[topBar][scrollIndex][tabBar]" options:0 metrics:nil views:views]];
    }
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
#ifdef STATUSBAR_HIDDEN
    [[UIApplication sharedApplication] setStatusBarHidden:YES];
#endif
    [self setTopBarHeight];
}

- (void)connectToPlaylist
{
    playlistCategoryConnection = IApp::instance()->selectedPlaylistConnector().connect([self](const pair<PlaylistCategory,shared_ptr<IPlaylist>> &p){

        auto &rhs = p.first;
        // -1 is invalid
        if (keepSettingsVisible || rhs.tag() < 0) {
            return;
        }
        
        SVNSLOG("selected connector: %@ %d", convertString(rhs.title()), rhs.tag());

    //	int prev_tag = _tabBar.prev_selected_tag;
    //	NSUInteger prev_index = _tabBar.prev_selected_index;
    //	int tag = rhs.tag();

        [self updateTabs];
        
        SVNSLOG(@"TABBAR: %d", rhs.tag());
        
        int prevTag = self.tabBar.selected_tag;

        [self.tabBar select_item_by_tag: rhs.tag()];
        //SVNSLOG(@"TABBAR2: %d %d", _tabBar.prev_selected_index, _tabBar.selected_index);
        
        shared_ptr<Gear::IPlaylist> playlistShown;
        if ([[controllers lastObject] isKindOfClass: SongListController.class]) {
            playlistShown = ((SongListController *)[controllers lastObject]).playlist;
        }
        const shared_ptr<IPlaylist> &playlist = p.second;
        
        transition_dir = TRANSITION_NONE;
        if (prevTag < rhs.tag()) {
            transition_dir = TRANSITION_FROM_RIGHT;
        } else if (prevTag > rhs.tag()) {
            transition_dir = TRANSITION_FROM_LEFT;
        } else if (_categoryLevel < rhs.level()) {
            transition_dir = TRANSITION_FROM_RIGHT;
        } else if (_categoryLevel > rhs.level()) {
            transition_dir = TRANSITION_FROM_LEFT;
        } else if (!firstAppearance) {
            
            //NSLog(@"playlist old: %@ new: %@", playlistShown ? convertString(playlistShown->playlistId()) : @"nothing",
            //                                  playlist ? convertString(playlist->playlistId()) : @"nothing");
            // same
            if (playlistShown && !playlist) {
                transition_dir = TRANSITION_FROM_LEFT;
            } else if (!playlistShown && playlist) {
                transition_dir = TRANSITION_FROM_RIGHT;
            } else if (playlistShown && playlistShown != playlist) {
                transition_dir = TRANSITION_FROM_RIGHT;
            } else {
                return;
            }
        }
        _categoryLevel = rhs.level();
                   
        firstAppearance = NO;

        // songlist
        if (playlist) {
            SVNSLOG(@"SONGLIST");
            
            [self switchToSongsList:NO];
        // list of playlists
        } else {
            if (prevTag == rhs.tag()) {
                SVNSLOG(@"SAME PLAYLIST");
                if ([[controllers lastObject] isKindOfClass: PlaylistListController.class]) {
                    SVNSLOG(@"SAME PLAYLIST TOP");
                    
                    if (transition_dir == TRANSITION_NONE) {
                        // we are already there
                        return;
                    }
                } else {
                    // we are already on the tab needed, just need to back out one step
                    transition_dir = TRANSITION_FROM_LEFT;
                }
            }
            SVNSLOG(@"SWitch to pl");
            [self switchToPlaylists];
        }
    });
}

- (void) addTransition {
	CATransition* transition = [CATransition animation];
	transition.duration = .25;
	transition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
	transition.type = kCATransitionPush;//kCATransitionReveal
	//kCATransitionReveal, kCATransitionMoveIn, kCATransitionPush,, kCATransitionFade
	transition.subtype = transition_dir == TRANSITION_FROM_RIGHT ? kCATransitionFromRight : kCATransitionFromLeft;

	transition.delegate = self;

	//kCATransitionFromTop, kCATransitionFromBottom, kCATransitionFromLeft, kCATransitionFromRight
    
//	[[_songListNaviController topViewController].navigationController.view.layer addAnimation:transition forKey:kCATransition];
	////_tabBar.collection_view.userInteractionEnabled = NO;
	[_songListNaviController.view.layer addAnimation: transition forKey:kCATransition];
}

- (void) animationDidStop: (CAAnimation *) animation finished: (BOOL) flag
{
	self.tabBar.collection_view.userInteractionEnabled = YES;
}

- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{
    // transitioning = YES must be much earlier!
    /*
    if (navigationController == self.songListNaviController && animated) {
        transitioning = YES;
    }*/
}

- (void)endAnimation
{
    transitioning = NO;
    if (pendingTransition) {
        //double delayInSeconds = 0.1f;
        //dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
        //dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        //});
        
        [self performTransition:pendingTransition];
        pendingTransition = nil;
    }
}

- (void)navigationController:(UINavigationController *)navigationController didShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{
    // '&& animated' would be wrong
    if (navigationController == self.songListNaviController) {
        
        //[self endAnimation];
    }
}

- (void)performTransition:(dispatch_block_t)block
{
    // we can't have two simultanous transitions (if the user taps too quickly)
    //NSLog(@"already: transitioning: %d", transitioning);
    
    if (!transitioning) {
        transitioning = YES;
        block();
        
        [self waitForEndTransition];
        
    } else {
        pendingTransition = block;
    }
}

- (void)waitForEndTransition
{
    // cleanest solution is to wait schedule ourselves instead of relying on hectic callbacks
    // of course it does not work at all when switching out of app
    
    double delayInSeconds = [AnimationUtility animationDuration] + 0.5f;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        if ([[UIApplication sharedApplication] applicationState] == UIApplicationStateActive) {
            [self endAnimation];
        } else {
            [self waitForEndTransition];
        }
    });
}

- (void)setController:(UIViewController *)controller
{
    NSArray *oldControllers = [controllers copy];
    
    controllers = [@[controller] mutableCopy];
    [self.songListNaviController addChildViewController:controller];
    [self.songListNaviController.view addSubview:controller.view];
    controller.view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    [self animateTransitionFrom:[oldControllers lastObject] to:controller onComplete:^{
        for (UIViewController *sub in oldControllers) {
            [sub.view removeFromSuperview];
            [sub removeFromParentViewController];
        }
    }];
}

- (void)pushController:(UIViewController *)controller
{
    [controllers addObject:controller];
    [self.songListNaviController addChildViewController:controller];
    [self.songListNaviController.view addSubview:controller.view];
}

- (void)popController
{
    UIViewController *controller = [controllers lastObject];
    [controllers removeLastObject];
    [controller removeFromParentViewController];
    [controller.view removeFromSuperview];
}

- (void) switchToPlaylists
{
	SVNSLOG(@"SWITCH PLAYLIST");
	if ([[controllers lastObject] isKindOfClass: PlaylistListController.class]) {
        //	 && ! [AppDelegate runningPreIOS7]) {
        
        SVNSLOG(@"TOP PLAY");
		UIViewController *vc = [controllers lastObject];
		((PlaylistListController *) controllers[0]).transition_dir = transition_dir;
		((PlaylistListController *) controllers[0]).snapshot_view = [AppDelegate runningPreIOS7] ? [AppDelegate snapshotFromView: vc.view] : [vc.view snapshotViewAfterScreenUpdates: YES];
	}
	/*
	else if ([AppDelegate runningPreIOS7]) {
		[self addTransition];
	}
*/
    PlaylistListController *playlistList = [[self storyboard] instantiateViewControllerWithIdentifier:@"playlistList"];
    playlistList.parentView = _songListNaviController.view;
    
    NSAssert([playlistList isKindOfClass:[PlaylistListController class]], @"root should be playlist list");
    [self setScrollIndex:[playlistList fastScrollIndex]];
    
    
    [self performTransition:^{
    	BOOL isPlayListOnTop = [[controllers lastObject] isKindOfClass: PlaylistListController.class];
        [self setController:playlistList];
        if ([AppDelegate runningPreIOS7] && !isPlayListOnTop) {
                SVNSLOG(@"TOP NOT PLAY");
            [self addTransition];
        }
    }];
}

- (void) switchToSongsList:(BOOL)push
{
#ifdef SPLASH_SCREEN
    return;
#endif
    
	SLNSLOG(@"SWITCH SONGLIST %d", transition_dir);
	
//    CGRect songListFrame = songListController.view.frame;
//    songListFrame.origin.y = kStatusBarHeight + kGearTopBarHeight;
//    songListController.view.frame = songListFrame;
//    
////	[songListController.navigationController pushViewController: songListController animated: YES];
    [self performTransition:^{
        SongListController *songListController = [[self storyboard] instantiateViewControllerWithIdentifier:@"songList"];
        
        songListController.parentView = _songListNaviController.view;
        
        if (push) {
            [self pushController:songListController];
        } else {
            [self setController:songListController];
        }
        if ([AppDelegate runningPreIOS7]) {
            [self addTransition];
        }
        [self setScrollIndex:songListController.fastScrollIndex];
    }];
////	songListController = nil;

///	if ([AppDelegate runningPreIOS7]) {
///		[self addTransition];
   /// }
}

- (BOOL)settingsShown
{
    return [[controllers lastObject] isKindOfClass:[SettingsViewController class]];
}

- (BOOL)settingsActive
{
    return keepSettingsVisible;
}

- (void)switchToSettings
{
    [self setScrollIndex:nil];
    
    // keep settings on screen until the user explicitly switches to somewhere else
    keepSettingsVisible = YES;
    
    [self performTransition:^{
        if (self.settingsShown) {

            NSUInteger n = controllers.count;
               
            if (n >= 2 && [controllers[n - 2] isKindOfClass:[SettingsViewController class]] ) {
             
                [self popController];
            }
            return;
        }

        SettingsViewController *settingsController = [[self storyboard] instantiateViewControllerWithIdentifier:@"settings"];
        settingsController.preferenceGroup = IApp::instance()->preferencesPanel()->preferenceGroups();
     
        //settingsController.view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        [self setController:settingsController];
        if ([AppDelegate runningPreIOS7]) {
            [self addTransition];
        }

    }];
     
}

- (void)tabBarDidSelectTag:(NSInteger)tag
{
//        NSUInteger set_index = [self.tabBar indexOfItem: item];
	//_tabBar.collection_view.userInteractionEnabled = NO;
    
    keepSettingsVisible = NO; 
    
	TBNSLOG(@"DIDS: %d", tag);

	if (tag == SETTINGS_TAG) {
		[self.tabBar select_item_by_tag:tag];
        transition_dir = TRANSITION_FROM_RIGHT;
		[self switchToSettings];
	} else {
		IApp::instance()->userSelectedCategory(tag);
	}
}

- (void)viewDidUnload
{
    [self setTabBar:nil];
    //[self setContentView:nil];
    [self setTabBar:nil];
    [super viewDidUnload];
}

+(UINavigationController*) topNavigationController
{
    return _topNavigationController;
}

#pragma mark transition stuff

- (id<UIViewControllerAnimatedTransitioning>)navigationController:(UINavigationController *)navigationController animationControllerForOperation:(UINavigationControllerOperation)operation fromViewController:(UIViewController *)fromVC toViewController:(UIViewController *)toVC {
  return (id <UIViewControllerAnimatedTransitioning>) self;

}

- (id <UIViewControllerAnimatedTransitioning>)animationControllerForPresentedController:(UIViewController *)presented presentingController:(UIViewController *)presenting sourceController:(UIViewController *)source {
	return (id <UIViewControllerAnimatedTransitioning>) self;
}

- (id <UIViewControllerAnimatedTransitioning>)animationControllerForDismissedController:(UIViewController *)dismissed {
	return nil;
}

- (id <UIViewControllerInteractiveTransitioning>)interactionControllerForPresentation:(id <UIViewControllerAnimatedTransitioning>)animator {
	return nil;
}

- (id <UIViewControllerInteractiveTransitioning>)interactionControllerForDismissal:(id <UIViewControllerAnimatedTransitioning>)animator {
	return nil;
}

- (NSTimeInterval) transitionDuration: (id <UIViewControllerContextTransitioning>) context
{
	return [AnimationUtility animationDuration];
}

// this is for the song list vs player screen navigation

- (void) animateTransition: (id <UIViewControllerContextTransitioning>) context
{
    //_tabBar.collection_view.userInteractionEnabled = NO;
    SVNSLOG(@"direction: %d", transition_dir);
    UIView *container = [context containerView];
    //container.backgroundColor = [UIColor colorWithRed: 10. / 255. green: 25. / 255. blue: 53. / 255. alpha: 1.];
    UIViewController *from_vc = (UIViewController *) [context viewControllerForKey:UITransitionContextFromViewControllerKey];
    UIViewController *to_vc = (UIViewController *) [context viewControllerForKey:UITransitionContextToViewControllerKey];
    [container addSubview: to_vc.view];
    
    CGRect target_rect = self.view.frame;
    CGRect from_rect = target_rect;
    CGRect to_rect = to_vc.view.frame;
    
    to_rect.origin.x = -320. * transition_dir;
    
    from_rect.origin.x = -to_rect.origin.x;
    to_vc.view.frame = to_rect;
    
    SVNSLOG(@"transition start");
    
    [AnimationUtility springAnimation:^{
        to_vc.view.frame = target_rect;
        from_vc.view.frame = from_rect;
    }
                           completion: ^(BOOL finished) {
                               [context completeTransition: YES];
                               //_tabBar.collection_view.userInteractionEnabled = YES;
                               SVNSLOG(@"transition end");
                               
                           }];
}


// this is for the song vs playlist list navigation
- (void)animateTransitionFrom:(UIViewController *)from_vc to:(UIViewController *)to_vc onComplete:(dispatch_block_t)onComplete
{
    UIView *container = self.slidingView;
    
    if (from_vc == nil) {
        to_vc.view.frame = container.bounds;
        onComplete();
    }
    
    if ([from_vc respondsToSelector:@selector(setPreventUpdates:)]) {
        [(id)from_vc setPreventUpdates:YES];
    }
    
    //_tabBar.collection_view.userInteractionEnabled = NO;
    SVNSLOG(@"direction: %d", transition_dir);
    //container.backgroundColor = [UIColor colorWithRed: 10. / 255. green: 25. / 255. blue: 53. / 255. alpha: 1.];
    //[container addSubview: to_vc.view];
    
    CGRect from_rect = from_vc.view.frame;
    CGRect to_rect = container.bounds;
    
    to_rect.origin.x = -container.frame.size.width * transition_dir;
    to_vc.view.frame = to_rect;

    from_rect.origin.x = -to_rect.origin.x;
    
    SVNSLOG(@"transition start");
    
    [AnimationUtility springAnimation:^{
        to_vc.view.frame = container.bounds;
        from_vc.view.frame = from_rect;
    }
    completion: ^(BOOL finished) {
        onComplete();
       //[context completeTransition: YES];
       //_tabBar.collection_view.userInteractionEnabled = YES;
       SVNSLOG(@"transition end");
       
    }];
}

- (void)showPlayer
{
    if (![[SongsViewController topNavigationController] isKindOfClass:PlayViewController.class]) {
        playerController = [[self storyboard] instantiateViewControllerWithIdentifier:@"playerController"];
        self.transition_dir = TRANSITION_FROM_RIGHT;
        [[SongsViewController topNavigationController] pushViewController:playerController animated:YES];
    }
}

- (void)applyTheme
{
    auto theme = IApp::instance()->themeManager()->current();
    
    self.tabBar.selected_color = theme->textHighlightColor();
    self.tabBar.unselected_color = theme->textColor();
    
    UIColor *fadeOutColor = Painter::convertColor(*dynamic_pointer_cast<Gui::Color>(theme->listBackground()));
    self.slidingView.backgroundColor = _contentView.backgroundColor = fadeOutColor;
    
    self.tabBar.background = theme->bottomBar();
    [self.tabBar applyTheme];
    
    [self.topBar applyTheme];
    [playerController applyTheme];
    
    for (id controller in controllers) {
        if ([controller respondsToSelector:@selector(applyTheme)]) {
            [controller applyTheme];
        }
    }
}

@end
