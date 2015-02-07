//
//  MainWindowController.h
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "StatusBarManager.h"
#import "ThemedWindow.h"
#include "ISong.h"
#include "SongEntry.h"

@class Song;
@class PlaylistManager;
@class SongsTableView;
@class SongAbstract;
@class AnimatedSplitView;
@class AlbumViewController;
@class ThemedSearchField;

extern NSString * const kPasteBoardTypeSong;

@interface MainWindowController : NSObject<ControlManagerDelegate, NSWindowDelegate, NSUserNotificationCenterDelegate, NSTableViewDataSource, NSTableViewDelegate,NSTextFieldDelegate>
@property (nonatomic,strong) IBOutlet SongsTableView *musicListView;
@property (nonatomic,strong) IBOutlet NSScrollView *musicListViewScroll;

@property (unsafe_unretained) IBOutlet PlaylistManager *playlistManager;

@property (unsafe_unretained) IBOutlet ThemedWindow *mainWindow;
@property (unsafe_unretained) IBOutlet NSTextField *songTitle;
@property (unsafe_unretained) IBOutlet NSTextField *songAlbumArtist;
@property (unsafe_unretained) IBOutlet NSTextField *songElapsed;
@property (unsafe_unretained) IBOutlet NSTextField *songRemaining;
@property (unsafe_unretained) IBOutlet NSSlider *playbackPositionSlider;
@property (unsafe_unretained) IBOutlet NSSlider *volumeSlider;
@property (unsafe_unretained) IBOutlet NSView *volumeHolder;

@property (nonatomic, strong) IBOutlet NSSearchField *searchField;
@property (nonatomic, strong) NSSearchField *searchFieldCustom;

@property (unsafe_unretained) IBOutlet NSButton *playButton;
@property (unsafe_unretained) IBOutlet NSButton *prevButton;
@property (unsafe_unretained) IBOutlet NSButton *nextButton;

@property (unsafe_unretained) IBOutlet NSButton *shuffleButton;
@property (unsafe_unretained) IBOutlet NSButton *repeatButton;
@property (unsafe_unretained) IBOutlet NSMenuItem *shuffleMenuItem;
@property (unsafe_unretained) IBOutlet NSMenuItem *repeatMenuItem;

@property (unsafe_unretained) IBOutlet NSButton *settingsButton;
@property (unsafe_unretained) IBOutlet NSButton *refreshButton;
@property (nonatomic,strong) IBOutlet NSView *topContainer;
@property (unsafe_unretained) IBOutlet NSImageView *albumArtImageWell;
@property (unsafe_unretained) IBOutlet NSView *albumArtHolder;
@property (unsafe_unretained) IBOutlet NSTextField *songCountLabel;
@property (unsafe_unretained) IBOutlet AnimatedSplitView *albumSplitView;
@property (unsafe_unretained) IBOutlet NSMenuItem *playMenuitem;
@property (unsafe_unretained) IBOutlet NSMenuItem *nextMenuitem;
@property (unsafe_unretained) IBOutlet NSMenuItem *prevMenuitem;
@property (unsafe_unretained) IBOutlet NSButton *shareButton;

@property (nonatomic, strong) NSMutableArray *visibleGroupings;
@property (unsafe_unretained) IBOutlet AlbumViewController *albumViewController;
@property (nonatomic, assign) BOOL albumViewVisible;
@property (unsafe_unretained) IBOutlet NSView *topBackground;
@property (unsafe_unretained) IBOutlet NSImageView *volumeIconMax;
@property (unsafe_unretained) IBOutlet NSImageView *volumeIcon;
@property (nonatomic,strong) IBOutlet NSTextField *bigMessageView;
@property (unsafe_unretained) IBOutlet NSView *imageBrowserView;
@property (unsafe_unretained) IBOutlet NSButton *outputsButton;
@property (nonatomic,strong) NSView *topView;
@property (nonatomic,assign) CGFloat translucencyOffset;


- (IBAction)outputsPressed:(id)sender;
- (IBAction)repeatPressed:(id)sender;
- (IBAction)positionChange:(id)sender;
- (IBAction)volumeChange:(id)sender;
- (IBAction)search:(NSSearchField *)sender;
- (IBAction)play:(id)sender;
- (IBAction)prev:(id)sender;
- (IBAction)next:(id)sender;
- (IBAction)refresh:(id)sender;
- (IBAction)jumpToSong:(id)sender;
- (IBAction)sendFeedback:(id)sender;
- (IBAction)shufflePressed:(id)sender;
- (void)playFirst;
- (void)playFirstForced:(BOOL)forced;
- (void)userInitiatedPlaySong:(Gear::SongEntry)songToPlay;
- (Gear::SongEntry)objectAtIndex:(long)index;

- (IBAction)focusSearch:(id)sender;

- (void)viewDidLoad;

- (IBAction)searchChanged:(id)sender;
- (std::shared_ptr<Gear::ISong>)currentlyPlaying;
- (void)storeLoaded;

- (void)adjustCollection;
- (void)applyTheme;

- (void)copySelectionToClipboard;

- (NSMenu *)tableView:(NSTableView *)tableView menuForRows:(NSIndexSet *)rows column:(NSUInteger)column;
- (void)applyTheme;
- (void)setConstraintsForNowPlayingShown:(BOOL)shown;

+ (void)refreshTableTextBackground:(NSView *)field row:(NSInteger)row table:(NSTableView *)table;

@end
