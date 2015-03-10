//
//  AppDelegate.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 6/23/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "AppDelegate.h"
#import <AVFoundation/AVFoundation.h>
#import "LockScreenController.h"
#include "App.h"
#include "IPreferencesPanel.h"

@implementation AppDelegate {
    __weak UIAlertView *sendUsageAlert;
}

using namespace Gear;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
#ifdef OPTION_DISABLE_INAPP
#ifdef DEBUG
    NSLog(@"Warning: in app purchases disabled");
#endif
    IPreferencesPanel::disableInApp();
#endif
    
    
    [NSThread setThreadPriority:1.0];
    
    // Override point for customization after application launch.
    IApp::registerInstance(App::instance());
    App::instance()->appStarted();
	[[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
    [[AVAudioSession sharedInstance] setDelegate:nil];
    
    [self setupAudioSession];
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	//[defaults setInteger: 0 forKey: @"RunCount"];
	//[defaults setBool: NO forKey: @"feedback_never"];
	//[defaults synchronize];

    
	NSInteger run_count = [defaults integerForKey: @"RunCount"];
	[defaults setInteger: ++run_count forKey: @"RunCount"];
	[defaults synchronize];
    
#ifdef DEBUG
//#define TEST_FEEDBACK
#endif
    
#ifdef TEST_FEEDBACK
    run_count = 5;
#endif
    
	if (! [defaults boolForKey: @"feedback_never"] && (! (run_count % 30) || run_count == 5 || run_count == 10)) {
		[[[UIAlertView alloc] initWithTitle: @"If you like Gear, please rate it or let us know how we could make it better!" message: nil delegate: self
			cancelButtonTitle: @"No, thanks" otherButtonTitles: @"Rate in App Store", @"Send feedback", @"Don't show again", nil] show];
    } else if ((run_count >= 2 && ![defaults boolForKey:@"AskedAboutStatistics"])) {
        
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Help us by sending anonymous usage data!" message:@"No personal data is collected. You can always opt out later in Settings." delegate:self cancelButtonTitle:@"No, thanks" otherButtonTitles:@"Yes, I'd like to help", nil];
        sendUsageAlert = alert;
        [alert show];
        
        [defaults setBool:YES forKey:@"AskedAboutStatistics"];
    }

	lockInfo = [[LockScreenController alloc] init];
    
    return YES;
}

- (void)sendFeedback
{
    [[UIApplication sharedApplication] openURL: [NSURL URLWithString:@"mailto:gearmusicplayer@gearmusicplayer.com?Subject=Gear%20iOS%20feedback&"]];
}

- (void) alertView: (UIAlertView *) alert_view clickedButtonAtIndex: (NSInteger) button_index
{
    if (alert_view == sendUsageAlert) {
        switch (button_index) {
            case 0:
                [alert_view dismissWithClickedButtonIndex: -1 animated: YES];
                break;
            case 1:
                [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"SendUsageStatistics"];
                break;
        }
        return;
    }
    
	switch (button_index) {
		case 0:
			[alert_view dismissWithClickedButtonIndex: -1 animated: YES];
			break;
		case 1:
			[[UIApplication sharedApplication] openURL: [NSURL URLWithString:@"https://itunes.apple.com/us/app/g-ear-player/id799375088?ls=1&mt=8"]];
			break;
		case 2:
			[self sendFeedback];
			break;
		case 3:
			NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
			[defaults setBool: YES forKey: @"feedback_never"];
			[defaults synchronize];
	}
}

static void audioInterruptionCallback(void     *inClientData,
                                      UInt32   inInterruptionState
                                      )
{
    // TODO: phone call and stuff
    //NSLog(@"interruption state: %d", (unsigned int)inInterruptionState);
}

- (void)setupAudioSession
{
    OSStatus status = AudioSessionInitialize(NULL, NULL, &audioInterruptionCallback, NULL);
    NSAssert(status == 0 || status == kAudioSessionAlreadyInitialized, @"audio init");
    
    UInt32 category = kAudioSessionCategory_MediaPlayback;
    status = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
    NSAssert(status == 0, @"audio category");
    
    status = AudioSessionSetActive(true);
    NSAssert(status == 0, @"audio active");
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


- (UIStoryboard *)storyBoard
{
    return [UIStoryboard storyboardWithName:@"MainStoryboard" bundle:nil];
}

+ (AppDelegate *)sharedDelegate;
{
    return [UIApplication sharedApplication].delegate;
}

+ (BOOL)runningPreIOS8
{
    return (floor(NSFoundationVersionNumber) <= 1047.25 /*NSFoundationVersionNumber_iOS_7_1*/);
}

+ (BOOL) runningPreIOS7 {
	return (floor(NSFoundationVersionNumber) <= NSFoundationVersionNumber_iOS_6_1);
}

+ (BOOL) runningPreIOS6 {
	return (floor(NSFoundationVersionNumber) < NSFoundationVersionNumber_iOS_6_0);
}

+ (BOOL) runningPreI5 {
	return (UIScreen.mainScreen.bounds.size.height < 567.);
}

+ (UIImageView *) snapshotFromView: (UIView *) view {
	UIGraphicsBeginImageContextWithOptions(view.bounds.size, YES, .0);
	[view.layer renderInContext: UIGraphicsGetCurrentContext()];
	UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
	UIGraphicsEndImageContext();
	return [[UIImageView alloc] initWithImage: image];
}

//extern "C" int OSMemoryNotificationCurrentLevel();

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
    // severity level is totally useless, reports 0 but can still kill our application
    //NSLog(@"memory warning severity: %d", OSMemoryNotificationCurrentLevel());
    IApp::instance()->freeUpMemory();
}

@end
