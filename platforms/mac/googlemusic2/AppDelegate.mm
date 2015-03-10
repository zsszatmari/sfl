//
//  AppDelegate.m
//  googlemusic2
//
//  Created by Zsolt Szatmári on 2/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <IOKit/hidsystem/ev_keymap.h>
#import "AppDelegate.h"
#import "MainWindowController.h"
#import "SongsTableView.h"
#import "PreferencesController.h"
#import "SPMediaKeyTap.h"
#import "AlbumViewController.h"
#import "Debug.h"
#import "ShortcutsController.h"
#import "MASShortcut.h"
#import "ThemeHelper.h"
#import "CocoaThemeManager.h"
#include "GearUtility.h"
#include "App.h"
#include "ServiceManager.h"
#include "IService.h"
#include "ISession.h"
#include "PlaybackController.h"
#import "Config.h"
#ifdef ENABLE_PADDLE
#import <Paddle/Paddle.h>
#import <Sparkle/Sparkle.h>
#import <HIDRemote.h>
#import "DDHidLib/DDHidAppleMikey.h"
#endif
#include "SessionManager.h"
#import "ThemedSplitView.h"
#include "LastFmController.h"
#include "StringUtility.h"
#import "GearApplication.h"
#import "PlaylistManager.h"

NSString *kUserDefaultsShowVolume = @"ShowVolume";
NSString *kUserDefaultsShowShareButton = @"ShowShareButton";
//NSString *kUserDefaultsHWControlsKey = @"AcceptsControlKeys";
NSString *kNotificationCenterEnabled = @"PostSongsToNotificationCenter";
NSString *kNotificationCenterEnabledOnlyLast = @"PostSongsToNotificationCenterOnlyLast";
NSString *kNotificationChangeForArrayController = @"ObjectsChangedInManagingContextPrivateNotificationGear";
NSString *kNotificationWillDelete = @"WillDeleteFromContext";

@interface AppDelegate () {
    NSMutableArray *songChunksToAdd;
    BOOL onlySaveUpdateOnEnd;
    
    NSMutableArray *gatheredSongs;
    
    SPMediaKeyTap *keyTap;
    BOOL mediaKeyAlreadyPressed;
    
    NSMutableArray *delayedNotifications;
    BOOL dispatching;
    
    Base::SignalConnection googleStateConnection;
    Base::SignalConnection refreshingConnection;
	BOOL albumArtHidden;
}

@end

@implementation AppDelegate

using namespace Gear;

@synthesize errorView = _errorView;
@synthesize errorScrollView = _errorScrollView;
@synthesize playlistsOutline = _playlistsOutline;

@synthesize window = _window;

@synthesize progressIndicator = _progressIndicator;
@synthesize feedbackButton = _feedbackButton;
@synthesize settingsButton = _settingsButton;
@synthesize preferencesController = _preferencesController;
@synthesize mainSplitView = _mainSplitView;
@synthesize albumArtImageWell = _albumArtImageWell;

@synthesize mainWindowController = _mainWindowController;
@synthesize preferencesPanel;
@synthesize preferencesConnectCell = _preferencesConnectCell;

+(void)initialize;
{
	if([self class] != [AppDelegate class]) return;
	
	// Register defaults for the whitelist of apps that want to use media keys
	[[NSUserDefaults standardUserDefaults] registerDefaults:[NSDictionary dictionaryWithObjectsAndKeys:
                                                             [SPMediaKeyTap defaultMediaKeyUserBundleIdentifiers], kMediaKeyUsingBundleIdentifiersDefaultsKey,
                                                             nil]];
}

+ (void)setupDefaults
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    // these must go before appStarted
    [defaults registerDefaults:[NSDictionary dictionaryWithObjectsAndKeys:
                                @14, @"ShowLastDaysCount",
                                @500, @"ShowLastItemsCount", nil]];
    
    
    // -----
    
    
    IApp::registerInstance(App::instance());
    Gear::localeInit();
    App::instance()->appStarted();
    
    NSArray *sortDescriptors = [SongsTableView sortDescriptorsForKey:[SongsTableView defaultKey] ascending:YES];
    NSData *transformedSortDescriptors = [[NSValueTransformer valueTransformerForName:NSKeyedUnarchiveFromDataTransformerName] reverseTransformedValue:sortDescriptors];
    
    // this must be called before there are initial values
    [self fixNonSqliteCompliantSortSelectors];
    
    //BOOL volumePresent = volumeObject != nil && [volumeObject intValue] < 100;
    
    NSMutableDictionary *initialValues = [[NSDictionary dictionaryWithObjectsAndKeys:
                                   transformedSortDescriptors,@"sortDescriptors",
                                   [NSNumber numberWithInt:100], @"volume",
                                   [NSNumber numberWithBool:YES], kUserDefaultsStatusBarKey,
                                  // [NSNumber numberWithBool:!lionOrBetter()],kUserDefaultsHWControlsKey,
                                   [NSNumber numberWithBool:YES], kNotificationCenterEnabled,
                                   @YES, @"PressingBackRewindsSong",
                                   @100, kDefaultsKeyAlbumCellSize,
                                   /*@(volumePresent)*/ @YES, kUserDefaultsShowVolume,
                                   [[MASShortcut shortcutWithKeyCode:kVK_ANSI_T modifierFlags:NSShiftKeyMask | NSControlKeyMask] data], kKeyShortcutThumbsUp,
                                   [[MASShortcut shortcutWithKeyCode:kVK_ANSI_G modifierFlags:NSShiftKeyMask | NSControlKeyMask] data], kKeyShortcutThumbsDown,
                                   [[MASShortcut shortcutWithKeyCode:kVK_ANSI_P modifierFlags:NSShiftKeyMask | NSControlKeyMask] data], kKeyShortcutPlayPause,
                                   [[MASShortcut shortcutWithKeyCode:kVK_ANSI_N modifierFlags:NSShiftKeyMask | NSControlKeyMask] data], kKeyShortcutNext,
                                   [[MASShortcut shortcutWithKeyCode:kVK_ANSI_B modifierFlags:NSShiftKeyMask | NSControlKeyMask] data], kKeyShortcutPrevious,
                                   [[MASShortcut shortcutWithKeyCode:kVK_ANSI_C modifierFlags:NSShiftKeyMask | NSControlKeyMask] data], kKeyShortcutJumpToCurrent,
                                   nil] mutableCopy];
    
    if (mountainLionOrBetter()) {
        [initialValues setObject:@YES forKey:kUserDefaultsShowShareButton];
    }
    
    if (lionOrBetter()) {
        [initialValues setObject:/*kThemeModernDark*/kThemeCocoa forKey:kUserDefaultsKeyTheme];
    } else {
        [initialValues setObject:kThemeCocoa forKey:kUserDefaultsKeyTheme];
    }
    [defaults registerDefaults:initialValues];
}

+ (void)fixNonSqliteCompliantSortSelectors
{
    NSString * const kKeySortDescriptors = @"sortDescriptors";
    
    id codedSortDescriptors = [[NSUserDefaults standardUserDefaults] objectForKey:kKeySortDescriptors];
    if (codedSortDescriptors == nil) {
        return;
    }
    NSArray *sortDescriptors = [[NSValueTransformer valueTransformerForName:NSKeyedUnarchiveFromDataTransformerName] transformedValue:codedSortDescriptors];
    NSMutableArray *newSortDescriptors = [NSMutableArray array];
    BOOL change = NO;
    
    for (NSSortDescriptor *sortDescriptor in sortDescriptors) {
        if ([sortDescriptor selector] == @selector(customCompare:) || [sortDescriptor selector] == @selector(localizedStandardCompare:) || [sortDescriptor selector] == @selector(discCompare:)) {
            change = YES;
            [newSortDescriptors addObject:[NSSortDescriptor sortDescriptorWithKey:sortDescriptor.key ascending:sortDescriptor.ascending]];
        } else {
            [newSortDescriptors addObject:sortDescriptor];
        }
    }
    if (change) {
        NSData *transformedSortDescriptors = [[NSValueTransformer valueTransformerForName:NSKeyedUnarchiveFromDataTransformerName] reverseTransformedValue:newSortDescriptors];
        [[NSUserDefaults standardUserDefaults]  setObject:transformedSortDescriptors forKey:kKeySortDescriptors];
    }
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication hasVisibleWindows:(BOOL)hasVisibleWindows
{
    if (hasVisibleWindows == NO) {
        [self.window makeKeyAndOrderFront:nil];
    }
    return YES;
}


- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMax ofSubviewAt:(NSInteger)dividerIndex
{
    NSInteger leftPanelMaxWidth = [[NSUserDefaults standardUserDefaults] integerForKey:@"LeftPanelMaxWidth"];
    if (leftPanelMaxWidth == 0) {
        leftPanelMaxWidth = 200;
    }
    return leftPanelMaxWidth;
}

- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)subview
{
    if (subview == [[splitView subviews] objectAtIndex:0]) {
        return NO;
    }
    return YES;
}

/*
- (void)setSelectedPlaylist:(id<PlaylistProtocol>)aSelectedPlaylist
{
    if (selectedPlaylist == aSelectedPlaylist) {
        [self.mainWindowController adjustCollection];
        return;
    }
    [self.mainWindowController saveFilter];
    selectedPlaylist = aSelectedPlaylist;
    [self.mainWindowController restoreFilter];
    [self.mainWindowController search:nil];
    [self.mainWindowController adjustCollection];
    [self.mainWindowController.playlistManager selectPlaylist:aSelectedPlaylist];
}*/

- (void)splitViewDidResizeSubviews:(NSNotification *)aNotification
{
    return;
    
    NSView *leftSplit = [[self.mainSplitView subviews] objectAtIndex:0];
    leftSplit = [[leftSplit subviews] objectAtIndex:0];  // because we have an extra holder see ThemedSplitView
    NSView *playlistOutlineContainer = [[leftSplit subviews] objectAtIndex:0];

    CGRect frame = playlistOutlineContainer.frame;

	if (albumArtHidden) {
		frame.origin.y = _nowPlayingImageTitle.frame.size.height;
		frame.size.height = leftSplit.frame.size.height - _nowPlayingImageTitle.frame.size.height;
		playlistOutlineContainer.frame = frame;

		NSRect artRect = self.albumArtHolder.frame;
		artRect.size.height = artRect.size.width + _nowPlayingImageTitle.frame.size.height + 2;
		artRect.origin.y = -artRect.size.height + _nowPlayingImageTitle.frame.size.height + 3;
		self.albumArtHolder.frame = artRect;

		frame = _albumArtImageWell.frame;
		frame.size.width = self.albumArtImageWell.superview.frame.size.width;
		frame.size.height = frame.size.width;
		_albumArtImageWell.frame = frame;

		//NSLog(@"RECT3: %@ %@", NSStringFromRect(frame), NSStringFromRect(self.albumArtHolder.frame));
	} else {

        CGRect albumArtHolderFrame = self.albumArtHolder.frame;
        frame.origin.y = albumArtHolderFrame.size.width + self.nowPlayingImageTitle.frame.size.height + 2;
        frame.size.height = leftSplit.frame.size.height - albumArtHolderFrame.size.width - self.nowPlayingImageTitle.frame.size.height - 2;
        playlistOutlineContainer.frame = frame;

        frame = self.albumArtHolder.frame;
        frame.size.height = frame.size.width + self.nowPlayingImageTitle.frame.size.height + 2;
        frame.origin.y = 0;
        self.albumArtHolder.frame = frame;

        NSRect titleRect = _nowPlayingImageTitle.frame;
        titleRect.size.width = frame.size.width;
        _nowPlayingImageTitle.frame = titleRect;

        // image well's right and top tends to shrink when making the left panel 0-sized then back
        const CGFloat kMargin = 0.0f;
        frame = self.albumArtImageWell.frame;
        frame.origin.x = kMargin;
        frame.origin.y = kMargin;
        frame.size.width = self.albumArtImageWell.superview.frame.size.width - 2 * kMargin;
        frame.size.height = frame.size.width;
        self.albumArtImageWell.frame = frame;
	}
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
    [self.mainWindowController viewDidLoad];
    
    // if not connecting
    
    // outdated
    /*if (IApp::instance()->serviceManager()->generalState() == IService::State::Offline) {
        [self.settingsButton setIntValue:YES];
        [self settingsPressed:self.settingsButton];
    }*/
}

#ifdef ENABLE_PADDLE

static io_connect_t get_event_driver(void)
{
    static  mach_port_t sEventDrvrRef = 0;
    mach_port_t masterPort, service, iter;
    kern_return_t    kr;
    
    if (!sEventDrvrRef)
    {
        // Get master device port
        kr = IOMasterPort( bootstrap_port, &masterPort );
        check( KERN_SUCCESS == kr);
        
        kr = IOServiceGetMatchingServices( masterPort, IOServiceMatching( kIOHIDSystemClass ), &iter );
        check( KERN_SUCCESS == kr);
        
        service = IOIteratorNext( iter );
        check( service );
        
        kr = IOServiceOpen( service, mach_task_self(),
                           kIOHIDParamConnectType, &sEventDrvrRef );
        check( KERN_SUCCESS == kr );
        
        IOObjectRelease( service );
        IOObjectRelease( iter );
    }
    return sEventDrvrRef;
}

static void HIDPostAuxKey( const UInt8 auxKeyCode, BOOL down )
{
    NXEventData   event;
    kern_return_t kr;
    IOGPoint      loc = { 0, 0 };
    
    // Key press event
    UInt32      evtInfo = auxKeyCode << 16 | (down ? (NX_KEYDOWN << 8) : (NX_KEYUP << 8));
    bzero(&event, sizeof(NXEventData));
    event.compound.subType = NX_SUBTYPE_AUX_CONTROL_BUTTONS;
    event.compound.misc.L[0] = evtInfo;
    kr = IOHIDPostEvent( get_event_driver(), NX_SYSDEFINED, loc, &event, kNXEventDataVersion, 0, FALSE );
    check( KERN_SUCCESS == kr );
}

- (void)hidRemote:(HIDRemote *)hidRemote
  eventWithButton:(HIDRemoteButtonCode)aButtonCode
        isPressed:(BOOL)aIsPressed
fromHardwareWithAttributes:(NSMutableDictionary *)attributes
{
    static BOOL hold;
    hold = (aButtonCode & kHIDRemoteButtonCodeHoldMask) != 0;
    static HIDRemoteButtonCode buttonCode;
    buttonCode = (HIDRemoteButtonCode)(aButtonCode & kHIDRemoteButtonCodeCodeMask);
    static BOOL isPressed;
    isPressed = aIsPressed;
    int keyCode;
    void (^soundEventSend)() = nil;
    switch(buttonCode) {
        case kHIDRemoteButtonCodeCenter:
        case kHIDRemoteButtonCodePlay:
            keyCode = NX_KEYTYPE_PLAY;
            break;
        case kHIDRemoteButtonCodeLeft:
            keyCode = NX_KEYTYPE_REWIND;
            break;
        case kHIDRemoteButtonCodeRight:
            keyCode = NX_KEYTYPE_FAST;
            break;
        case kHIDRemoteButtonCodeUp:
#ifdef DEBUG_REMOTEVOLUME
            //NSLog(@"MagicKeys volume initiating up %d", isPressed);
#endif
            soundEventSend = ^{
#ifdef DEBUG_REMOTEVOLUME
                //NSLog(@"MagicKeys volume sending up %d", isPressed);
#endif
                HIDPostAuxKey(NX_KEYTYPE_SOUND_UP, isPressed);
            };
            break;
        case kHIDRemoteButtonCodeDown:
#ifdef DEBUG_REMOTEVOLUME
            //NSLog(@"MagicKeys volume initiating down %d", isPressed);
#endif
            soundEventSend = ^{
#ifdef DEBUG_REMOTEVOLUME
                //NSLog(@"MagicKeys volume sending down %d", isPressed);
#endif
                HIDPostAuxKey(NX_KEYTYPE_SOUND_DOWN, isPressed);
            };
            break;;
        default:
            // not interested
            return;
    }
    
    if (soundEventSend != nil) {
        if (hold) {
            
            dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
                                                             0, 0, dispatch_get_current_queue());
            if (timer)
            {
                dispatch_source_set_timer(timer, dispatch_walltime(NULL, 0), 0.1f * NSEC_PER_SEC, 0.01f * NSEC_PER_SEC);
                dispatch_source_set_event_handler(timer, ^{
                    if (isPressed) {
                        // pressed still
                        soundEventSend();
                    } else {
                        dispatch_release(timer);
                    }
                });
                dispatch_resume(timer);
            }
        } else {
            soundEventSend();
        }
        return;
    }
    
    BOOL isRepeat = NO;
    if (hold) {
        isRepeat = YES;
    }
    
    [self simulateKeyPress:keyCode pressed:isPressed repeat:isRepeat];
}

- (void)simulateKeyPress:(int)keyCode pressed:(BOOL)isPressed repeat:(BOOL)isRepeat
{
    // some voodoo here
    NSInteger data = (keyCode << 16) | (isPressed ? (0xA << 8): 0) | (isRepeat ? 0x1 : 0);
    
    NSTimeInterval timestamp = [[NSProcessInfo processInfo] systemUptime];
    NSEvent *event = [NSEvent otherEventWithType:NSSystemDefined location:CGPointMake(0, 0) modifierFlags:0 timestamp:timestamp windowNumber:0 context:0 subtype:SPSystemDefinedEventMediaKeys data1:data data2:0];
    [self mediaKeyTap:nil receivedMediaKeyEvent:event];
}

- (void) ddhidAppleMikey: (DDHidAppleMikey *) mikey
                   press: (unsigned) usageId
                upOrDown:(BOOL)down
{
    const int kVolumeDown = 141;
    const int kVolumeUp = 140;
    const int kPlayPause = 137;
    
    if (!down) {
        return;
    }
    switch (usageId) {
        case kVolumeDown:
            HIDPostAuxKey(NX_KEYTYPE_SOUND_DOWN, YES);
            HIDPostAuxKey(NX_KEYTYPE_SOUND_DOWN, NO);
            break;
        case kVolumeUp:
            HIDPostAuxKey(NX_KEYTYPE_SOUND_UP, YES);
            HIDPostAuxKey(NX_KEYTYPE_SOUND_UP, NO);
            break;
        case kPlayPause:
            // this is wrong becuase it also launches iTunes
            //HIDPostAuxKey(NX_KEYTYPE_PLAY, YES);
            //HIDPostAuxKey(NX_KEYTYPE_PLAY, NO);
            
            [self simulateKeyPress:NX_KEYTYPE_PLAY pressed:YES repeat:NO];
            [self simulateKeyPress:NX_KEYTYPE_PLAY pressed:NO repeat:NO];
            break;
    }
}

#endif

#ifdef ENABLE_PADDLE
- (void)scheduleMessage
{
    if (![[Paddle sharedInstance] productActivated]) {
        // half an hour is not good, let it be one day!
        double delayInSeconds = 24 * 60 * 60;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
            if (![[Paddle sharedInstance] productActivated]) {
                [[Paddle sharedInstance] showLicencing];
                [self scheduleMessage];
            }
        });
    }
}
#endif

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    NSAssert([[[NSApplication sharedApplication] class] isEqual:[GearApplication class]], @"wrong nsapplication instance");
    
    [[[[AppDelegate sharedDelegate] mainWindowController] playlistManager] setupCategories];
    
	_nowPlayingImageTitle.delegate = self;
#ifdef ENABLE_PADDLE
#ifndef DISABLE_PADDLE
    NSDictionary *productInfo = [NSDictionary dictionaryWithObjectsAndKeys:
                                 @"5.99", kPADCurrentPrice,
                                 @"Treasure Box", kPADDevName,
                                 @"USD", kPADCurrency,
                                 //@"http://www.macupdate.com/util/iconlg/17227.png", kPADImage,
                                 @"G-Ear Player", kPADProductName,
                                 @"7", kPADTrialDuration,
                                 @"Thanks for trying out Gear! If you like it, please Buy or Enter Licence to avoid seeing messages like this.",
                                 kPADTrialText,
                                 @"G-Ear.iconset", kPADProductImage, //Image file in your project
                                 nil];
    
    NSWindow *window = nil;
    [[Paddle sharedInstance] startLicensing:@"45433b6f757849a91bccd988ba3c5df8" vendorId:@"401" productId:@"161"
                                  timeTrial:YES productInfo:productInfo withWindow:window];
    
    {
        NSMenu *menu = [[[[NSApplication sharedApplication] mainMenu] itemAtIndex:0] submenu];
        SUUpdater *updater = [SUUpdater sharedUpdater];
        
        NSMenuItem *item = [menu insertItemWithTitle:@"Check for Updates..." action:@selector(checkForUpdates:) keyEquivalent:@"" atIndex:1];
        [item setTarget:updater];
        [updater checkForUpdatesInBackground];
        
        if (![[Paddle sharedInstance] productActivated]) {
            NSMenuItem *item = [menu insertItemWithTitle:@"Activate..." action:@selector(showLicencing) keyEquivalent:@"" atIndex:2];
            [item setTarget:[Paddle sharedInstance]];
        }
    }
    
    static HIDRemote *hidRemote = [HIDRemote sharedHIDRemote];
    [hidRemote setDelegate:(id)self];
    
    static DDHidAppleMikey *mikey;
    NSArray *mikeys = [DDHidAppleMikey allMikeys];
    if ([mikeys count] > 0) {
        mikey = [mikeys objectAtIndex:0];
        [mikey setDelegate:self];
        [mikey setListenInExclusiveMode:YES];
        @try {
            [mikey startListening];
        } @catch (id exception) {
            NSLog(@"access to mic control failed");
        }
    }
    
    [self scheduleMessage];
#endif
#endif
    
    {
        // playback menu
        
        NSMenu *menu = [[[[NSApplication sharedApplication] mainMenu] itemAtIndex:3] submenu];
        [menu addItem:[NSMenuItem separatorItem]];
        NSMenuItem *offlineModeMenuItem = [menu addItemWithTitle:@"Offline Mode" action:@selector(toggleOfflineMode:) keyEquivalent:@"o"];
        [self setOfflineMenuItemChecked:offlineModeMenuItem];
        [offlineModeMenuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
    }
    
    self.progressIndicator.hidden = YES;
    
   // [self.mainSplitView set]
    [self.mainSplitView setDelegate:self];
    CGFloat maxSplit = [self splitView:nil constrainMaxCoordinate:0 ofSubviewAt:0];
    if ([[[self.mainSplitView subviews] objectAtIndex:0] frame].size.width > maxSplit) {
        [self.mainSplitView setPosition:maxSplit ofDividerAtIndex:0];
    }
    [self splitViewDidResizeSubviews:nil];
    
    keyTap = [[SPMediaKeyTap alloc] initWithDelegate:self];
	if([SPMediaKeyTap usesGlobalMediaKeyTap]) {
        // a.k.a snow leopard
		[keyTap startWatchingMediaKeys];
    }
    
    delayedNotifications = [NSMutableArray array];
    
    //[self.credentialsController initLogin];
    
//oldlogin    [preferencesPanel setDefaultButtonCell:self.preferencesConnectCell];
    
    [self manageRunCount];
    
    [self.preferencesController viewDidLoad];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self showWhatsNew];
    });
/*oldlogin
    googleStateConnection = IApp::instance()->serviceManager()->googlePlayService()->stateConnector().connect([self](const IService::State state){
       
        switch(state) {
            case IService::State::Offline:
                [self.googleConnectLabel setTitle:@"Log In"];
                [self.googleConnectLabel setEnabled:YES];
                break;
            case IService::State::Online:
                [self.googleConnectLabel setTitle:@"Log Out"];
                [self.googleConnectLabel setEnabled:YES];
                break;
            case IService::State::Connecting:
                [self.googleConnectLabel setTitle:@"Connecting..."];
                [self.googleConnectLabel setEnabled:NO];
                break;
        }
        
        [self updateProgress];
    });
    
    refreshingConnection = IApp::instance()->sessionManager()->refreshingConnector().connect([self](const bool vale){
        
        [self updateProgress];
    });
*/    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW,0), ^{
        
        NSFileManager *fm = [NSFileManager defaultManager];
        NSURL *appSupportDirectory = [fm URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
        if (appSupportDirectory != nil) {
            for (NSURL *url in [fm enumeratorAtURL:appSupportDirectory includingPropertiesForKeys:@[NSURLIsDirectoryKey] options:0 errorHandler:nil]) {
                
                if ([[url lastPathComponent] hasPrefix:@"googlemusic-account-"]/* || [[url lastPathComponent] hasPrefix:@"googlemusic-"]*/) {
                    NSNumber *isDir = nil;
                    [url getResourceValue:&isDir forKey:NSURLIsDirectoryKey error:nil];
                    if (isDir && ![isDir boolValue]) {
                        [fm removeItemAtURL:url error:nil];
                    }
                }
            }
        }
    });
    
//#define SERVICE_TEST_TITLE @"Youtube test"
//#define SERVICE_TEST_NUM 1
    
//#define SERVICE_TEST_TITLE @"Spotify test"
//#define SERVICE_TEST_NUM 3

#ifdef SERVICE_TEST_TITLE
    // just a temporary youtube test... don't leave these here in the long term! really, I mean it.
    auto service = IApp::instance()->serviceManager()->services().at(SERVICE_TEST_NUM);
    //service->autoconnect();
    [self.youtubeCheckBox setTitle:SERVICE_TEST_TITLE];
    [self.youtubeCheckBox setIntValue:(service->state() != IService::State::Offline)];
    [self.youtubeCheckBox setTarget:self];
    [self.youtubeCheckBox setAction:@selector(youtubePressed:)];
#endif 
    
    // this is needed here, in main.m it does not perform well (menu problems)
    if (![[[NSUserDefaults standardUserDefaults] objectForKey:@"statusbarMode"] boolValue]){
        
        [AppDelegate showInDock];
    }
    
    if (![[NSUserDefaults standardUserDefaults] boolForKey:@"NotFirstRun"]) {
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NotFirstRun"];
        [self.preferencesController showWindow];
    }
    
    [self lastFmInit];
}

static shared_ptr<IService> offlineService()
{
    auto services = IApp::instance()->serviceManager()->services();
    for (auto s : services) {
        if (s->serviceIdentifier() == "offline") {
            return s;
        }
    }
    return nullptr;
}

- (void)setOfflineMenuItemChecked:(NSMenuItem *)item
{
    [item setState:offlineService()->state() == IService::State::Online];
}

- (void)toggleOfflineMode:(NSMenuItem *)sender
{
    auto s = offlineService();
    if (s->state() == IService::State::Online) {
        s->disconnect();
    } else {
        s->connect();
    }
    [self setOfflineMenuItemChecked:sender];
}

- (void)manageRunCount
{
    NSInteger runCount = [[NSUserDefaults standardUserDefaults] integerForKey:@"RunCount"];
    ++runCount;
    [[NSUserDefaults standardUserDefaults] setInteger:runCount forKey:@"RunCount"];
    
    if (NSClassFromString(@"NSPopover") != nil) {
        
        // don't popover on snow leo, but count!
#ifndef ENABLE_PADDLE
        BOOL remotePopup = NO;
        if (runCount > 0) {
            // someone already has used g-ear
            if (![[NSUserDefaults standardUserDefaults] boolForKey:@"AppleHeadphonesVersionRan"]) {
                
                remotePopup = YES;
                [self showAppleRemotePopup];
            }
        }
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"AppleHeadphonesVersionRan"];
        if (remotePopup) {
            return;
        }
#endif
        
        if (runCount == 4 || runCount == 10 || ((runCount % 30) == 0)) {
            
            NSString *text;
            if (runCount >= 10) {
                text = @"Please tell us your opinion so that we can make an even greater product for You! If you are satisfied, Like, Follow, +1 us or write a review! If you are not, drop an email!";
            } else {
                text = @"Please tell us your opinion so that we can make an even greater product for You! If you are satisfied, Like, Follow or +1 us! If you are not, drop an email!";
            }
            CGSize size = NSMakeSize(250, 60);
            NSMutableAttributedString *attributed = [[NSMutableAttributedString alloc] initWithString:text];
            void(^setURLForText)(NSString *, NSString *) = ^(NSString *url, NSString *pattern){
                NSRange range = [text rangeOfString:pattern];
                if (range.location == NSNotFound) {
                    return;
                }
                [attributed setAttributes:[NSDictionary dictionaryWithObject:[NSURL URLWithString:url] forKey:NSLinkAttributeName] range:range];
            };
            setURLForText(@"http://www.facebook.com/pages/G-Ear/371979526180078",@"Like");
            setURLForText(@"http://www.twitter.com/_TreasureBox_",@"Follow");
            setURLForText(@"https://plus.google.com/100660399948019918631",@"+1");
            // this is not valid anymore
            //            setURLForText(@"https://www.paddle.com/software/490953#disqus_thread",@"review");
            setURLForText(@"http://itunes.apple.com/us/app/g-ear/id513751032?ls=1&mt=12",@"review");
            setURLForText(@"mailto:zsolt.szatmari@me.com?Subject=Gear%20Feedback&Body=Dear%20Treasure%20Box%21%0A%0A%20%20I%20like%20your%20product%20but/and...%0A%20%20I%20don%27t%20like%20G-Ear%20because...%0A%20%20I%27ve%20found%20some%20bugs%2C%20that%20is..%0A%20%20I%20have%20some%20suggestions..%0A%20%20May%20I%20request%20the%20following%20feature..%3F%0A%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%0ABest%20luck%2C%0A%0A",@"email");
            
            [self showPopoverWithText:attributed withSize:size fromView:self.feedbackButton];
            
        }
    }
}

- (NSView *)albumArtHolder
{
    return self.mainWindowController.albumArtHolder;
}

- (void)setAlbumArtHolder:(id)value
{
    // do nothing, this is just a placeholder for the xib, but we relay ourselves instead
}

- (NSString *) showHide: (NSString *) title
{

	NSView *leftSplit = [[self.mainSplitView subviews][0] subviews][0];
	
	if ([title isEqualToString: @"Show"]) {

		[NSAnimationContext beginGrouping];
		[[NSAnimationContext currentContext] setDuration: .4];
		[[_albumArtImageWell animator] setAlphaValue: 1.];
		[NSAnimationContext endGrouping];
		albumArtHidden = NO;
        [self.mainWindowController setConstraintsForNowPlayingShown:YES];
		return @"Hide";
	} else {
        
		[NSAnimationContext beginGrouping];
		[[NSAnimationContext currentContext] setDuration: .4];
		[[_albumArtImageWell animator] setAlphaValue: .0];
		[NSAnimationContext endGrouping];
		albumArtHidden = YES;
        [self.mainWindowController setConstraintsForNowPlayingShown:NO];
		return @"Show";
	}
}

- (IBAction)youtubePressed:(id)sender
{
#ifdef SERVICE_TEST_TITLE
    auto service = IApp::instance()->serviceManager()->services().at(SERVICE_TEST_NUM);
    if ([sender intValue]) {
        service->connect();
    } else {
        service->disconnect();
    }
#endif
}

- (void)updateProgress
{
    bool refreshing = IApp::instance()->sessionManager()->refreshingConnector().value();
    bool connecting = IApp::instance()->serviceManager()->googlePlayService()->stateConnector().value() == IService::State::Connecting;
    
    if (refreshing || connecting) {
        [self startProgress];
    } else {
        [self endProgress];
    }
}

- (void)showAppleRemotePopup
{
    NSString *text = @"There is a new version of Magic Keys available, supporting Apple headphones!";
    NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:text];
    NSRange range = [text rangeOfString:@"Magic Keys"];
    [string setAttributes:[NSDictionary dictionaryWithObject:[NSURL URLWithString:@"http://www.gearmusicplayer.com/extras.html"] forKey:NSLinkAttributeName] range:range];
    [self showPopoverWithText:string withSize:CGSizeMake(180,45) fromView:self.settingsButton];

}

- (void)showPopoverWithText:(NSAttributedString *)attributed withSize:(CGSize)size fromView:(NSView *)view
{
    double delayInSeconds = 5.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        
        NSPopover *popover = [[NSPopover alloc] init];
        
        NSViewController *viewController = [[NSViewController alloc] init];
        NSTextView *field = [[NSTextView alloc] init];

        [[field textStorage] setAttributedString:attributed];
        
        [field setEditable:NO];
        [field setDrawsBackground:NO];
        [field setAlignment:NSCenterTextAlignment];
        
        viewController.view = field;
        popover.contentViewController = viewController;
        popover.contentSize = size;
        popover.behavior = NSPopoverBehaviorSemitransient;
        
        [popover showRelativeToRect:view.frame ofView:self.window.contentView preferredEdge:NSMaxYEdge];
    });
}

- (BOOL)magicKeysRunning
{
#ifdef DEBUG_TEST_MAGICKEYS
    return YES;
#endif
    
    NSArray *apps = [[NSWorkspace sharedWorkspace] runningApplications];
    
    for (NSRunningApplication *app in apps) {
        NSString *bundle = [app bundleIdentifier];
        if ([bundle isEqualToString:@"com.treasurebox.MagicKeys-Agent"]) {
            return YES;
        }
    }
    return NO;
}

- (void)mediaKeyTap:(SPMediaKeyTap*)keyTap receivedMediaKeyEvent:(NSEvent*)event
{
    
	// here be dragons...
	int keyCode = (([event data1] & 0xFFFF0000) >> 16);
	int keyFlags = ([event data1] & 0x0000FFFF);
	BOOL keyIsPressed = (((keyFlags & 0xFF00) >> 8)) == 0xA;
	int keyRepeat = (keyFlags & 0x1);
    
    if (keyCode != NX_KEYTYPE_PLAY && keyCode != NX_KEYTYPE_FAST && keyCode != NX_KEYTYPE_REWIND
                                   && keyCode != NX_KEYTYPE_NEXT && keyCode != NX_KEYTYPE_PREVIOUS){
        return;
    }
    
    static int lastKeyCode;
    static BOOL wasRepeat;
    BOOL continous = NO;
    BOOL notInterested = NO;
    if (lastKeyCode == keyCode && !wasRepeat && !keyIsPressed) {
        // just released
    } else if (lastKeyCode == keyCode && keyRepeat && keyIsPressed) {
        // holding
        continous = YES;
    } else if (lastKeyCode == keyCode && wasRepeat && !keyIsPressed) {
        // was holding but released
        continous = YES;
    } else {
        notInterested = YES;
    }
    lastKeyCode = keyCode;
    wasRepeat = keyRepeat;
    
    if (continous) {
        // intermittently, fast forward/backward is disabled 
        notInterested = YES;
    }
    
    if (notInterested) {
        return;
    }

    
    //BOOL wasEverPressed = mediaKeyAlreadyPressed;
    mediaKeyAlreadyPressed = YES;
#ifndef ENABLE_PADDLE
    if (lionOrBetter() && ![self magicKeysRunning]) {
        
        if (/*!wasEverPressed && */ lionOrBetter()) {
            NSPopover *popover = [[NSPopover alloc] init];
            
            NSViewController *viewController = [[NSViewController alloc] init];
            NSTextView *field = [[NSTextView alloc] init];
            NSString *string = @"To use media keys or\nan Apple Remote with Gear,\nyou'll need to install ";
      
            NSMutableAttributedString *attrString = [[NSMutableAttributedString alloc] initWithString:string];
            NSAttributedString *magicKeys = [AppDelegate hyperlinkFromString:@"Magic Keys" withURL:[NSURL URLWithString:@"http://www.gearmusicplayer.com/extras.html"]];
            [attrString appendAttributedString:magicKeys];
            [attrString appendAttributedString:[[NSAttributedString alloc] initWithString:@"!"]];
            
            [[field textStorage] setAttributedString:attrString];
            [field setEditable:NO];
            [field setDrawsBackground:NO];
            [field setAlignment:NSCenterTextAlignment];        
            
            viewController.view = field;
            popover.contentViewController = viewController;
            popover.contentSize = NSMakeSize(188, 42);
            popover.behavior = NSPopoverBehaviorTransient;
            
            [popover showRelativeToRect:self.settingsButton.frame ofView:self.window.contentView preferredEdge:NSMaxYEdge];
            

        }
        return;
    }
#endif

    const float kFastPlaySpeed = 10.0f;
    
    switch (keyCode) {
        case NX_KEYTYPE_PLAY:
            [[self.mainWindowController playButton] performClick:nil]; 
            break;
        case NX_KEYTYPE_FAST:
        case NX_KEYTYPE_NEXT:
            if (!continous) {
                [[self.mainWindowController nextButton] performClick:nil];
            } else {
                if (keyIsPressed) {
                    [self setSpeedIfPlaying:kFastPlaySpeed];
                } else {
                    [self setSpeedIfPlaying:1.0f];
                }
            }
            break;
        case NX_KEYTYPE_REWIND:
        case NX_KEYTYPE_PREVIOUS:
            if (!continous) {
                [[self.mainWindowController prevButton] performClick:nil];
            } else {
                if (keyIsPressed) {
                    [self setSpeedIfPlaying:-1.0f * kFastPlaySpeed];
                } else {
                    [self setSpeedIfPlaying:1.0f];
                }
            }
            break;
        default:
            break;
            // More cases defined in hidsystem/ev_keymap.h
    }

}

- (void)setSpeedIfPlaying:(float)speed
{
    PlaybackController::instance()->setRateIfPlaying(speed);
}

/*
- (void)clearSongs
{
    // fetch...
    NSEntityDescription *entity = [NSEntityDescription entityForName:@"Song" inManagedObjectContext:self.remoteContextWorker];
    NSFetchRequest *fetchAll = [[NSFetchRequest alloc] init];
    [fetchAll setEntity:entity];
    
    NSArray *songs = [self.remoteContextWorker executeFetchRequest:fetchAll error:nil];
    
    for (NSManagedObject *song in songs) {
        [self.remoteContextWorker deleteObject:song];
    }

}

- (void)clearSongsNow
{
    [self performRemoteContextWorker:^{
        [self clearSongs];
        [self.remoteContextWorker saveImproved:nil];
    }];
}*/

- (void)refreshList
{
    if (IApp::instance()->sessionManager()->refreshingConnector().value()) {
        return;
    }
    
    [self showErrorMessage:nil];
    
    IApp::instance()->sessionManager()->refresh();
}

- (IBAction)settingsPressed:(id)sender {
    if ([sender intValue]) {
        [self.preferencesController showWindow];
        [preferencesPanel makeKeyAndOrderFront:sender];
    } else {
        [self.preferencesController hideWindow];
    }
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}


// Returns the directory the application uses to store the Core Data store file. This code uses a directory named "zsoltt.googlemusic2" in the user's Application Support directory.
- (NSURL *)applicationFilesDirectory
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *appSupportURL = [[fileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] lastObject];
    return [appSupportURL URLByAppendingPathComponent:@"com.treasurebox.gear"];
}

// Returns the NSUndoManager for the application. In this case, the manager returned is that of the managed object context for the application.
- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window
{
    return nil;
    //return [[self managedObjectContext] undoManager];
}

// Performs the save action for the application, which is to send the save: message to the application's managed object context. Any encountered errors are presented to the user.
- (IBAction)saveAction:(id)sender
{
    /*NSError *error = nil;
    
    if (![[self managedObjectContext] commitEditing]) {
        NSLog(@"%@:%@ unable to commit editing before saving", [self class], NSStringFromSelector(_cmd));
    }
    
    if (![[self managedObjectContext] save:&error]) {
        [[NSApplication sharedApplication] presentError:error];
    }*/
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    return NSTerminateNow;
}

+ (AppDelegate *)sharedDelegate
{
    return (AppDelegate *)[NSApplication sharedApplication].delegate;
}

- (void)startProgress
{
    [self.progressIndicator startAnimation:nil];
    [self.progressIndicator setHidden:NO];
}

- (void)endProgress
{
    [self.progressIndicator stopAnimation:nil];
    [self.progressIndicator setHidden:YES];
}

- (void)doConnect
{
    IApp::instance()->serviceManager()->googlePlayService()->connect();
}

- (void)doDisconnect
{
    IApp::instance()->serviceManager()->googlePlayService()->disconnect();
}

- (BOOL)isLoggedIn
{
    return IApp::instance()->serviceManager()->googlePlayService()->state() == IService::State::Online;
}

- (IBAction)connect:(id)sender
{
#ifndef DISABLE_PADDLE
#ifdef ENABLE_PADDLE
    if (![[Paddle sharedInstance] productActivated] && [[[Paddle sharedInstance] daysRemainingOnTrial] integerValue] == 0) {
        return;
    }
#endif
#endif
    
    if (![self isLoggedIn]) {
        
        [self doConnect];
    } else {
        [self doDisconnect];
    }

}

- (IBAction)refresh:(id)sender {
    [self refreshList];
    
#ifdef DEBUG
    //[self showErrorMessage:[[NSAttributedString alloc] initWithString:@"muuuu"]];;
#endif
}

- (void)showErrorMessage:(NSAttributedString *)message
{
    if (message != nil) {
        
        [self endProgress];        
        
        [[self.errorView textStorage] setAttributedString:message];
        [self.errorView setAlignment:NSCenterTextAlignment];
        
        [self.errorScrollView setHidden:NO];
        [self.tableSuperview setHidden:YES];
    } else {
        [self.errorScrollView setHidden:YES];
        [self.tableSuperview setHidden:NO];
    }
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
    IApp::instance()->didBecomeActive();
}

- (void)copy:(id)sender
{
    [self.mainWindowController copySelectionToClipboard];
}

- (void)doShowWhatsNew:(id)sender
{
    [self.whatsNewWindow setTitle:@"Gear - What's New"];
    [self.whatsNewWindow setLevel:NSFloatingWindowLevel];
    [self.whatsNewWindow makeKeyAndOrderFront:nil];
}

- (IBAction)openWhatsNewLink:(id)sender
{
    [self.whatsNewWindow orderOut:self];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://itunes.apple.com/us/app/g-ear-player/id799375088?ls=1&mt=8"]];
}

- (IBAction)themeChanged:(id)sender
{
    [CocoaThemeManager refreshTheme];
    [self.mainWindowController applyTheme];
    
    [self splitViewDidResizeSubviews:nil];
}

- (void)showWhatsNew
{
    //return;
    
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSInteger runCount = [[NSUserDefaults standardUserDefaults] integerForKey:@"RunCount"];
    
#if DEBUG
    if (![defaults boolForKey:@"AskedAboutStatistics"] && runCount > 1) {
    //if (YES) {
#else
    if (![defaults boolForKey:@"AskedAboutStatistics"] && runCount > 1) {
#endif
        // os x bug: if going to full screen on start, and trying to do modal window while animating, the whole app just hangs (but no beachball)
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            
            NSAlert *alert = [[NSAlert alloc] init];
            
            // reviewer didn't like this
            //[alert setShowsSuppressionButton:YES];
            //[[alert suppressionButton] setTitle:@"Allow sending anonymous usage statistics"];
            //[[alert suppressionButton] setIntValue:1];
            
            alert.messageText = @"Help us by sending anonymous usage data!";
            alert.informativeText = @"No personal data is collected. You can always opt out later in Settings.\n\nCollected information consists of system configuration (e.g. version of operating system, screen size), statistics about how Gear is used (e.g. number of songs, playlists, specific feature-usage), and will be uploaded to a remote server.\n\nWe will only use this data to make Gear better.";
            
            [alert addButtonWithTitle:@"Yes, I'd like to help"];
            [alert addButtonWithTitle:@"No, thanks"];
            
            NSInteger selectedButton = [alert runModal];
            
            [defaults setBool:(selectedButton == NSAlertFirstButtonReturn) forKey:@"SendUsageStatistics"];
            
            
            [defaults setBool:YES forKey:@"AskedAboutStatistics"];
        });
        return;
    }
    
    
    NSString *version = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
	NSString *prevVersion = [defaults objectForKey:@"prevVersion"];
    
    if ([prevVersion intValue] >= 379) {
        [defaults setObject:@(YES) forKey:@"ShownIPhoneGear"];
    }
  
    if ([defaults boolForKey:@"ShownIPhoneGear"]) {
        return;
    }
    
    BOOL whatsNewNeeded = [prevVersion intValue]<[version intValue];

    if (whatsNewNeeded) {
        [self doShowWhatsNew:nil];
    }
    
    [defaults setObject:version forKey:@"prevVersion"];
    [defaults setObject:@(YES) forKey:@"ShownIPhoneGear"];
    [defaults synchronize];
}

+ (NSAttributedString *)hyperlinkFromString:(NSString*)inString withURL:(NSURL*)aURL
{
    NSMutableAttributedString* attrString = [[NSMutableAttributedString alloc] initWithString: inString];
    NSRange range = NSMakeRange(0, [attrString length]);
    
    [attrString beginEditing];
    [attrString addAttribute:NSLinkAttributeName value:[aURL absoluteString] range:range];
    
    // make the text appear in blue
    [attrString addAttribute:NSForegroundColorAttributeName value:[NSColor blueColor] range:range];
    
    // next make the text appear with an underline
    [attrString addAttribute:
     NSUnderlineStyleAttributeName value:[NSNumber numberWithInt:NSSingleUnderlineStyle] range:range];
    
    [attrString endEditing];
    
    return attrString;
}

+ (void)showInDock
{
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    SetFrontProcessWithOptions(&psn, kSetFrontProcessCausedByUser);
}

- (void)lastFmInit
{
    auto controller = IApp::instance()->lastFmController();
    static auto messageConnection = controller->messageConnector().connect([=](const std::string &value){
    
        [self.lastFmMessage setStringValue:convertString(value)];
    });
    static auto checkboxConnection = controller->enabledConnector().connect([=](bool value){

        [self.lastFmCheckBox setState:value];
    });
}

- (IBAction)lastFmAction:(NSButton *)sender
{
    auto controller = IApp::instance()->lastFmController();
    controller->setEnabled([sender state]);
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
    NSMenu *menu = [[NSMenu alloc] init];
    
    [menu addItem:[self.mainWindowController.playMenuitem copy]];
    [menu addItem:[self.mainWindowController.nextMenuitem copy]];
    [menu addItem:[self.mainWindowController.prevMenuitem copy]];
    
    return menu;
}

- (BOOL)application:(NSApplication *)sender delegateHandlesKey:(NSString *)key
{
    if ([key isEqualToString: @"bridgedObjects"]) {
        return YES;
    }
    return NO;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    IApp::instance()->terminate();
    
    // we kill ourselves to escape crashing because of system deallocating static variables
    exit(0);
}

@end

BOOL lionOrBetter()
{
    return floor(NSAppKitVersionNumber) >= NSAppKitVersionNumber10_7;
}

BOOL mountainLionOrBetter()
{
    return [NSSharingService class] != nil;
}

BOOL mavericksOrBetter()
{
    NSApplication *app = [NSApplication sharedApplication];
    return [app respondsToSelector:@selector(occlusionState)];
}

BOOL yosemiteOrBetter() {
	return floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_9;
}
    
BOOL vibrancyEnabled()
{
    return ![[NSUserDefaults standardUserDefaults] boolForKey:@"ReduceTransparency"];
}