//
//  AppDelegate.h
//  googlemusic2
//
//  Created by Zsolt Szatmári on 2/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <WebKit/Webkit.h>
#import "YRKSpinningProgressIndicator.h"
#import "NowPlayingImageTitle.h"

@class MainWindowController;
@class GoogleMusicSession;
@class PreferencesController;
@class SPMediaKeyTap;
@class PlaylistManager;
@class Playlist;
@class ThemedSplitView;
@protocol PlaylistProtocol;

extern NSString *kUserDefaultsShowVolume;
extern NSString *kUserDefaultsShowShareButton;
extern NSString *kNotificationCenterEnabled;
extern NSString *kNotificationCenterEnabledOnlyLast;
extern NSString *kNotificationChangeForArrayController;
extern NSString *kNotificationWillDelete;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSSplitViewDelegate, ShowHideDelegate>
@property (unsafe_unretained) IBOutlet NSTextField *lastFmMessage;
@property (unsafe_unretained) IBOutlet NSButton *lastFmCheckBox;

@property (assign) IBOutlet NSWindow *window;

- (IBAction)saveAction:(id)sender;
// do not use this tableView, get it from MainWindowController instead, if you must
@property (nonatomic,weak) IBOutlet NSTableView *tableView;
@property (unsafe_unretained) IBOutlet NSTableView *playlistsOutline;
@property (unsafe_unretained) IBOutlet NowPlayingImageTitle *nowPlayingImageTitle;
@property (unsafe_unretained) IBOutlet NSPanel *preferencesPanel;
@property (unsafe_unretained) IBOutlet NSButtonCell *preferencesConnectCell;
@property (unsafe_unretained) IBOutlet YRKSpinningProgressIndicator *progressIndicator;
@property (unsafe_unretained) IBOutlet NSButton *feedbackButton;
@property (unsafe_unretained) IBOutlet NSButton *settingsButton;
@property (unsafe_unretained) IBOutlet NSButton *eqButton;
@property (unsafe_unretained) IBOutlet PreferencesController *preferencesController;
@property (unsafe_unretained) IBOutlet ThemedSplitView *mainSplitView;
@property (unsafe_unretained) IBOutlet NSImageView *albumArtImageWell;
@property (unsafe_unretained) IBOutlet NSWindow *whatsNewWindow;
@property (unsafe_unretained) IBOutlet NSButton *googleConnectLabel;

@property (unsafe_unretained) IBOutlet MainWindowController *mainWindowController;
+ (AppDelegate *)sharedDelegate;

@property (unsafe_unretained) IBOutlet NSView *tableSuperview;
@property (unsafe_unretained) IBOutlet NSTextView *errorView;
@property (unsafe_unretained) IBOutlet NSScrollView *errorScrollView;
@property (unsafe_unretained) IBOutlet NSButton *youtubeCheckBox;

- (void)refreshList;
- (IBAction)refresh:(id)sender;
- (IBAction)settingsPressed:(id)sender;
- (IBAction)connect:(id)sender;
- (void)showErrorMessage:(NSAttributedString *)message;
- (void)mediaKeyTap:(SPMediaKeyTap*)keyTap receivedMediaKeyEvent:(NSEvent*)event;

- (void)endProgress;
- (BOOL)isLoggedIn;
- (void)doConnect;
+ (void)setupDefaults;
- (IBAction)doShowWhatsNew:(id)sender;
- (IBAction)openWhatsNewLink:(id)sender;
- (IBAction)themeChanged:(id)sender;
+ (NSAttributedString *)hyperlinkFromString:(NSString*)inString withURL:(NSURL*)aURL;
+ (void)showInDock;
- (IBAction)lastFmAction:(id)sender;

@end

#ifdef __cplusplus
extern "C" {
#endif
    
BOOL lionOrBetter();
BOOL mountainLionOrBetter();
BOOL mavericksOrBetter();
BOOL yosemiteOrBetter();
BOOL vibrancyEnabled();
#ifdef __cplusplus
}
#endif
