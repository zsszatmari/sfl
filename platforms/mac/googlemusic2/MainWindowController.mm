    //
//  MainWindowController.m
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <AppKit/NSKeyValueBinding.h>
#include <iostream>

#import "MainWindowController.h"
#import "AppDelegate.h"
#import "StatusBarManager.h"
#import "ShareManager.h"
#import "PlaylistManager.h"
#import "RatingCell.h"
#import "LastFmController.h"
#import "MillisecToTimeTransformer.h"
#import "GearSlider.h"
#import "NSImage+Tinting.h"
#import "SongsTableView.h"
#import "CalendarTimeTransformer.h"
#import "NumberIfNotZeroTransformer.h"
#import "ArrayCountTransformer.h"
#import "InstantCell.h"
#import "AnimatedSplitView.h"
#import "Grouping.h"
#import "AlbumViewController.h"
#import "Debug.h"
#import "CocoaThemeManager.h"
#import "NSImage+Tinting.h"
#import "RectButtonCell.h"
#import "CorrectedButtonCell.h"
#import "HighlightingTextField.h"
#import "RoundedButtonCell.h"
#import "ScrollViewWorkaround.h"
#import "Config.h"
#ifdef ENABLE_PADDLE
#import <Paddle/Paddle.h>
#endif
#include "StringUtility.h"
#include "SortDescriptorConverter.h"
#include "App.h"
#include "SongEntry.h"
#include "AlbumArtStash.h"
#include "PromisedImage.h"
#include "Painter.h"
#include "ISongIntent.h"
#include "PlaybackController.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "Writer.h"
#import "OutputsController.h"
#include "Chain.h"
#include "ValidPtr.h"
#import "ColoredTextFieldCell.h"
#include "IPlaylist.h"
#import "ThemedSplitView.h"
#import "GearCommand.h"
#import "SongListTextFieldCell.h"
#import "TextImageFieldCell.h"
#include "SessionManager.h"
#include "OfflineState.h"
#import "SourceView.h"
#import "ColoredView.h"
#import "CustomSearchField.h"
#import "GradientView.h"
#import "GearSliderCell.h"
#import "VerticallyCenteredTextField.h"
#import "GearVisualEffectView.h"
#import "SongsRowView.h"
#include "Rules.h"
#import "ThemedHeaderCell.h"
#import "NSView+Recursive.h"
#import "FKPrelude.h"
#import "AlignedTextField.h"
#include "sfl/Map.h"
#import "NaviBarView.h"

using namespace Gear;
using namespace Gui;

static NSString * const kSingleSongLoopPossible = @"SingleSongLoopPossible";
NSString * const kPasteBoardTypeSong = @"com.treasurebox.gear.song";



//#define HIDE_CURRENTINSTANT_COLUMN

@interface MainWindowController () {
    SongsTableView *musicListView;
    
    SongEntry cachedSong;
    
    NSString *searchCategory;
    
    StatusBarManager *statusBarManager;
    ShareManager *shareManager;
    shared_ptr<ISong> lastNotificationSong;
    
    shared_ptr<ISongArray> visibleEntries;
    SongView _visibleSongs;
    
    CGFloat userInitiatedScrollOrigin;
    BOOL skipJumpNextTime;
    
    NSMutableSet *observedSongs;
    
    NSPredicate *userFilterPredicate;
    
    NSCache *filtersForPlaylists;
    NSCache *searchCategoriesForPlaylists;
    
    NSMenu *cachedSearchMenu;
    
    vector<SongEntry> contextMenuSongs;
    NSMutableArray *addToMenuPlaylists;

    BOOL fullscreened;
    NSMutableArray *columnsShouldBeHidden;
    BOOL playFirstPending;
    BOOL ignoreSearch;
    
    SignalConnection selectedPlaylistConnection;
    shared_ptr<PromisedImage> displayAlbum;
    SignalConnection displayAlbumConnection;
    SignalConnection visibleUpdateConnection;
    
    SignalConnection playedSongConnection;
    SignalConnection playingConnection;
    SignalConnection repeatConnection;
    SignalConnection shuffleConnection;
    
    SignalConnection elapsedConnection;
    SignalConnection remainingConnection;
    SignalConnection ratioConnection;
    
    NSTimeInterval lastScroll;
    
    BOOL deselectOnNextReload;
	NSMutableArray *downloadIndicatorArray;
    
    NSView *albumArtColorView;
    
    NSArray *constraintsNowPlayingShown;
    NSArray *constraintsNowPlayingHidden;
    NSDictionary *constraintsNowPlayingViews;
    CGFloat nowPlayingHeight;
    NSLayoutConstraint *nowPlayingHeightConstraint;
    BOOL nowPlayingShown;
    
    NSArray *gradients;
    NSIndexSet *selectionReload;
    ColoredView *lineAtBottomOfTopBorder;
    
    std::map<NSView *,std::pair<NSLayoutConstraint *,NSLayoutConstraint *>> marginConstraints;
    
    //NSTimer *searchTimer;
    
    NaviBarView *naviBarView;
}

- (void)setupToggleColumnMenu;

@end

@interface MenuItemWithIntent : NSMenuItem
@property(nonatomic, assign) shared_ptr<ISongIntent> intent;
@end

@implementation MenuItemWithIntent
@synthesize intent;
@end

@implementation MainWindowController
@synthesize playlistManager;
@synthesize mainWindow;
@synthesize songTitle;
@synthesize songAlbumArtist;
@synthesize songElapsed;
@synthesize songRemaining;
@synthesize playbackPositionSlider;
@synthesize volumeSlider;
@synthesize searchField;
@synthesize playButton;
@synthesize prevButton;
@synthesize nextButton;
@synthesize shuffleButton;
@synthesize repeatButton;
@synthesize settingsButton;
@synthesize refreshButton;
@synthesize topContainer;
@synthesize albumArtImageWell;
@synthesize albumArtHolder;
@synthesize songCountLabel;
@synthesize musicListViewScroll;
@synthesize visibleGroupings;
@synthesize albumViewVisible;
@synthesize topView;
@synthesize bigMessageView;
@synthesize translucencyOffset;
@synthesize volumeIcon;
@synthesize volumeIconMax;
@synthesize albumSplitView;

- (id)init
{
    self = [super init];
    if (self) {
         [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self 
                                                               selector: @selector(pauseSong:) 
                                                                   name: NSWorkspaceWillSleepNotification object: NULL];
        
        for (NSString *notificationName in @[@"com.apple.screenIsLocked", @"com.apple.sessionDidMoveOffConsole"]) {
            [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(screenLocked:) name:notificationName object:nil suspensionBehavior:NSNotificationSuspensionBehaviorDeliverImmediately];
        }
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(changeForArrayController:) name:kNotificationChangeForArrayController object:nil];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateRepeatMixPossible:) name:NSUserDefaultsDidChangeNotification object:nil];
        
        observedSongs = [NSMutableSet set];
        
#ifdef ENABLE_PADDLE
        [[NSNotificationCenter defaultCenter] addObserverForName:kPADActivated object:nil queue:nil usingBlock:^(NSNotification *note) {
            [self reloadTable];
        }];
#endif
    }
    return self;
}

static const int kTopHeight = 48;

// original: 11/20
static const float kEnlargeTableTextRatio = 1.12f;

//static const CGFloat kEnlargeTableText = 1.35f;
//static const int kEnlargedTableTextVerticalOffset = -3;
static const CGFloat kEnlargeTableText = 11 * kEnlargeTableTextRatio - 11;
static const int kEnlargeTableTextRow = 20 * kEnlargeTableTextRatio - 20;
static const int kEnlargedTableTextVerticalOffset = -3;

- (void)awakeFromNib
{
    //NSLog(@"enlarge: %f", kEnlargeTableText);
    //[self.mainWindow.contentView setWantsLayer:YES];
    
    //if ([[NSScreen mainScreen] backingScaleFactor] < 1.99f) {
        // musiclistviewscroll: don't apply to retina, according to Marius this did cause scrollbars to appear...
        // hopefully the clipview does not suffer from the same
    
        // an another problem is that apparently calayer trickery causes rare symptoms of 'views freezing' on the top bar
        //[self.musicListView.superview setWantsLayer:YES];
    //}
        
    [self setTopHeight:kTopHeight];
    
    [self.musicListView setRowHeight:[self.musicListView rowHeight] + kEnlargeTableTextRow];
    
    [OutputsController sharedController];
    /*if (!lionOrBetter()) {
        [self.outputsButton removeFromSuperview];
        self.outputsButton = nil;
    } */   

    NSAssert(_repeatMenuItem != nil, @"repeat menu item unreachable");
    NSAssert(_shuffleMenuItem != nil, @"shuffle menu item unreachable");
    [_repeatMenuItem setTarget:self];
    [_shuffleMenuItem setTarget:self];
    [_repeatMenuItem setAction:@selector(repeatPressed:)];
    [_shuffleMenuItem setAction:@selector(shufflePressed:)];
    
    if (!lionOrBetter()) {
        [self.searchField setFocusRingType:NSFocusRingTypeNone];
    }
    
	downloadIndicatorArray = [[NSMutableArray alloc] initWithCapacity: 10];
    
    
    Class vibrantClass = NSClassFromString(@"NSVisualEffectView");
    if (!vibrancyEnabled()) {
        vibrantClass = nil;
    }
    if (vibrantClass)
    {
        NSVisualEffectView *vibrant=[[vibrantClass alloc] initWithFrame:self.albumArtHolder.frame];
        [vibrant setAutoresizingMask:self.albumArtHolder.autoresizingMask];
        [vibrant setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
        //[vibrant setState:NSVisualEffectStateInactive];
        [self.albumArtHolder.superview addSubview:vibrant];
        
        albumArtColorView = [[ColoredView alloc] initWithFrame:self.albumArtHolder.frame];
        albumArtColorView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        [vibrant addSubview:albumArtColorView];
        
        NSView *content = ((NSScrollView *)self.albumArtHolder).documentView;
        [self.albumArtHolder removeFromSuperview];
        [vibrant addSubview:content];
        self.albumArtHolder = vibrant;
    } else {
        albumArtColorView = self.albumArtHolder;
    }
    //((NSScrollView *)albumArtHolder).contentView.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    CGRect topFrame = self.topContainer.frame;
    topFrame.origin.x = self.topContainer.superview.bounds.origin.x;
    topFrame.size.width = self.topContainer.superview.bounds.size.width;
    //topFrame.origin.x = 0;
    //topFrame.size.width = self.mainWindow.frame.size.width;
    if (vibrantClass) {
        NSVisualEffectView *vibrant=[[GearVisualEffectView alloc] initWithFrame:topFrame];
        
        [vibrant setBlendingMode:NSVisualEffectBlendingModeWithinWindow];
        //[vibrant setMaterial:NSVisualEffectMaterialLight];
        [vibrant setMaterial:NSVisualEffectMaterialTitlebar];
        [vibrant setWantsLayer:YES];
        topView = vibrant;
        
        // let's reach beyond the stars (till the top of the window)
        translucencyOffset = kTopHeight;
        NSView *splitView = [AppDelegate sharedDelegate].mainSplitView;
        CGRect frame = splitView.frame;
        frame.size.height += translucencyOffset;
        splitView.frame = frame;
        
        
        [self setMusicListContentInsets];
        
    } else {
        topView = [[NSView alloc] initWithFrame:topFrame];
    }
    [topView setAutoresizingMask:self.topContainer.autoresizingMask];
    [self.topContainer.superview addSubview:topView];
    [self.topContainer removeFromSuperview];
    
    self.topContainer.translatesAutoresizingMaskIntoConstraints = NO;
    [topView addSubview:self.topContainer];
    [topView addSubview:self.shareButton];
    
    // otherwise animatedsplitview won't work correctly...
    bigMessageView.translatesAutoresizingMaskIntoConstraints = NO;
    [self.bigMessageView.superview addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[bigMessageView]|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(bigMessageView)]];
    [self.bigMessageView.superview addConstraint:[NSLayoutConstraint constraintWithItem:self.bigMessageView attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self.bigMessageView.superview attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    
    
    // this is important for the modern dark theme
    // we need two instances: the top one needs to be added inside the
    GradientView *gradientTop = [[GradientView alloc] init];
    gradientTop.drawTop = YES;
    gradientTop.translatesAutoresizingMaskIntoConstraints = NO;
    [topView addSubview:gradientTop positioned:NSWindowBelow relativeTo:nil];
    NSView *gradientBottom = [[GradientView alloc] initWithFrame:[self.mainWindow.contentView bounds]];
    [gradientBottom setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [self.mainWindow.contentView addSubview:gradientBottom positioned:NSWindowBelow relativeTo:nil];
    gradients = @[gradientTop, gradientBottom];

    {
        NSDictionary *views = NSDictionaryOfVariableBindings(topContainer,gradientTop);
        [topView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[topContainer]|" options:0 metrics:nil views:views]];
        [topView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[gradientTop]|" options:0 metrics:nil views:views]];
        [topView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[gradientTop]|" options:0 metrics:nil views:views]];
    }
    
    // we need to have two search fields, because if we override the cell's drawrect, even if we don't do anything else but
    // call super, we don't get nice yosemite behaviour
    self.searchFieldCustom = [[CustomSearchField alloc] initWithFrame:self.searchField.frame];
    self.searchFieldCustom.autoresizingMask = self.searchField.autoresizingMask;
    [self.searchField.superview addSubview:self.searchFieldCustom];
    
    // remove 'Comment' field from search menu template as it does not make sense in the current form
    [self.searchField.searchMenuTemplate removeItem:[self.searchField.searchMenuTemplate.itemArray lastObject]];
    
    self.searchFieldCustom.searchMenuTemplate = self.searchField.searchMenuTemplate;
    [self.searchFieldCustom setTarget:self.searchField.target];
    [self.searchFieldCustom setAction:self.searchField.action];
    [self.searchFieldCustom.cell setUsesSingleLineMode:YES];
    [self.searchFieldCustom.cell setScrollable:YES];
    [self.searchFieldCustom setContinuous:YES];
    
    
    dispatch_async(dispatch_get_main_queue(), ^{
        NSView *leftView = [[AppDelegate sharedDelegate].mainSplitView leftView];
        NSView *playlist = self.playlistManager.scrollView;
        NSView *nowPlayingTitle = [AppDelegate sharedDelegate].nowPlayingImageTitle;
        NSImageView *albumArt = [AppDelegate sharedDelegate].albumArtImageWell;
        
        for (NSView *view in [[leftView subviews] copy]) {
            [view removeFromSuperview];
        }
        NSDictionary *views = NSDictionaryOfVariableBindings(playlist, nowPlayingTitle, albumArt, albumArtHolder);
        constraintsNowPlayingViews = views;
        for (NSView *view in [views allValues]) {
            view.translatesAutoresizingMaskIntoConstraints = NO;
        }
        [leftView addSubview:playlist];
        [leftView addSubview:albumArtHolder];
        [leftView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[playlist]|" options:0 metrics:nil views:views]];
        [leftView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[albumArtHolder]|" options:0 metrics:nil views:views]];
        [leftView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[playlist][albumArtHolder]" options:0 metrics:nil views:views]];
        
        [self setConstraintsForNowPlayingShown:YES];
        
        NSView *innerHolder = [nowPlayingTitle superview];
        
        albumArt.imageScaling = NSImageScaleProportionallyUpOrDown;
        [innerHolder addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[nowPlayingTitle]|" options:0 metrics:nil views:views]];
        [innerHolder addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[albumArt]|" options:0 metrics:nil views:views]];
        [innerHolder addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[nowPlayingTitle][albumArt]|" options:0 metrics:nil views:views]];
        [innerHolder addConstraint:[NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:albumArt attribute:NSLayoutAttributeWidth multiplier:1 constant:0]];
        
        
    });
    
    [(GearSliderCell *)self.volumeSlider.cell setStyle:@"volumeslider"];
    [(GearSliderCell *)self.playbackPositionSlider.cell setStyle:@"seekslider"];
    for (id view in [self.mainWindow.contentView subviews]) {
        if ([view isKindOfClass:[NSSlider class]]) {
            GearSliderCell *albumSizeCell = [view cell];
            [albumSizeCell setStyle:@"albumsizeslider"];
            break;
        }
    }
    
    VerticallyCenteredTextField *(^centerTextField)(NSTextField *) = ^(NSTextField *original){
        VerticallyCenteredTextField *clone;
        if ([original isKindOfClass:[HighlightingTextField class]]) {
            clone = [[HighlightingTextField alloc] init];
        } else {
            clone = [[VerticallyCenteredTextField alloc] initWithFrame:original.frame];
        }
        clone.editable = original.editable;
        clone.drawsBackground = original.drawsBackground;
        clone.translatesAutoresizingMaskIntoConstraints = NO;
        clone.alignment = original.alignment;
        clone.bezeled = original.bezeled;
        clone.action = original.action;
        clone.target = original.target;
        clone.wantsLayer = YES;
        
        [original.superview addSubview:clone];
        [original removeFromSuperview];
        return clone;
    };
    self.songElapsed = centerTextField(self.songElapsed);
    self.songRemaining = centerTextField(self.songRemaining);
    self.songTitle = [centerTextField(self.songTitle) alignBottom];
    self.songAlbumArtist = [centerTextField(self.songAlbumArtist) alignTop];
    [self.songTitle setContentCompressionResistancePriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationHorizontal];
    [self.songTitle setContentHuggingPriority:1 forOrientation:NSLayoutConstraintOrientationHorizontal];
    [self.songAlbumArtist setContentCompressionResistancePriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationHorizontal];
    [self.songAlbumArtist setContentHuggingPriority:1 forOrientation:NSLayoutConstraintOrientationHorizontal];
    
    NSView *songTitleLeft = [[NSView alloc] init];
    songTitleLeft.translatesAutoresizingMaskIntoConstraints = NO;
    [topContainer addSubview:songTitleLeft];
    NSView *songTitleRight = [[NSView alloc] init];
    songTitleRight.translatesAutoresizingMaskIntoConstraints = NO;
    [topContainer addSubview:songTitleRight];
    
    NSDictionary *views = NSDictionaryOfVariableBindings(songElapsed,songRemaining,playbackPositionSlider,songRemaining,songTitle,songAlbumArtist,volumeIcon,volumeIconMax,songTitleLeft,songTitleRight,searchField,volumeSlider);
    // depends on font
    CGFloat separator = 5;
    [topContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[songElapsed(==40)]-sep-[playbackPositionSlider]-sep-[songRemaining(==40)]" options:NSLayoutFormatAlignAllCenterY metrics:@{@"sep":@(separator)} views:views]];
    
    searchField.translatesAutoresizingMaskIntoConstraints = NO;
    self.searchFieldCustom.translatesAutoresizingMaskIntoConstraints = NO;
    volumeIcon.translatesAutoresizingMaskIntoConstraints = NO;
    volumeIconMax.translatesAutoresizingMaskIntoConstraints = NO;
    [topContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[volumeIcon]-4-[volumeSlider]-4-[volumeIconMax]" options:0 metrics:nil views:views]];
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:volumeIcon attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:volumeSlider attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:volumeIconMax attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:volumeSlider attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [topContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[volumeIconMax][songTitleLeft(>=36)][songTitle][songTitleRight(>=36)][songElapsed]" options:0 metrics:nil views:views]];
    [topContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[searchField(==147)]-|" options:0 metrics:nil views:views]];
    
    [topContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[volumeIconMax][songTitleLeft][songAlbumArtist][songTitleRight][songElapsed]" options:0 metrics:nil views:views]];
    //[topContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[volumeIconMax]-(>=36)-[songAlbumArtist]-(>=36)-[songElapsed]" options:0 metrics:nil views:views]];
    
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:songTitleLeft attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:songTitleRight attribute:NSLayoutAttributeWidth multiplier:1 constant:14]];
    //NSLayoutConstraint *constraint;
    //constraint = [NSLayoutConstraint layoutCot]
    //[topContainer addConstraint:[NSLayoutConstraint constr]]
    
    using namespace sfl;
    vector<NSView *> centerItems;
    centerItems.push_back(volumeSlider);
    centerItems.push_back(self.prevButton);
    centerItems.push_back(self.playButton);
    centerItems.push_back(self.nextButton);
    for (auto &p : zip(centerItems,tail(centerItems))) {
        [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:p.first attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:p.second attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    }
    
    const CGFloat attraction = 1;
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:songTitle attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:playbackPositionSlider attribute:NSLayoutAttributeCenterY multiplier:1 constant:attraction]];
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:songAlbumArtist attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:playbackPositionSlider attribute:NSLayoutAttributeCenterY multiplier:1 constant:-attraction]];
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:searchField attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:playbackPositionSlider attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:self.searchFieldCustom attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:playbackPositionSlider attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:self.searchFieldCustom attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:searchField attribute:NSLayoutAttributeLeft multiplier:1 constant:0]];
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:self.searchFieldCustom attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:searchField attribute:NSLayoutAttributeRight multiplier:1 constant:0]];
    
    
    
    
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:songTitle attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1 constant:30]];
    [topContainer addConstraint:[NSLayoutConstraint constraintWithItem:songAlbumArtist attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1 constant:30]];
     
    
    NSMenu *playbackMenu = [[[[NSApplication sharedApplication] mainMenu] itemAtIndex:3] submenu];
    [playbackMenu addItem:[NSMenuItem separatorItem]];
    [[playbackMenu addItemWithTitle:@"Jump to Now Playing" action:@selector(jumpToSong:) keyEquivalent:@"l"] setKeyEquivalentModifierMask:NSCommandKeyMask];
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:@"FullScreen"]) {
        [self.mainWindow toggleFullScreen:nil];
    }
    
    // already called from applyTheme
    //[self recreateMusicListView];
    
    naviBarView = [[NaviBarView alloc] init];
    naviBarView.translatesAutoresizingMaskIntoConstraints = NO;
    NSView *splitViewSuper = self.albumSplitView.superview;
    [splitViewSuper addSubview:naviBarView];
    
    NSDictionary *naviViews = NSDictionaryOfVariableBindings(naviBarView,albumSplitView,topContainer);

    [splitViewSuper addSubview:naviBarView];
    [splitViewSuper addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[naviBarView]|" options:0 metrics:nil views:naviViews]];
    [splitViewSuper addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[naviBarView(==24)]" options:0 metrics:nil views:naviViews]];
    [self.mainWindow.contentView addConstraint:[NSLayoutConstraint constraintWithItem:naviBarView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:topContainer attribute:NSLayoutAttributeBottom multiplier:1 constant:0]];
}

- (void)setConstraintsForNowPlayingShown:(BOOL)shown
{
    if (constraintsNowPlayingViews == nil) {
        return;
    }
    
    nowPlayingShown = shown;
    NSView *leftView = [[AppDelegate sharedDelegate].mainSplitView leftView];
    
    if (constraintsNowPlayingShown) {
        [leftView removeConstraints:constraintsNowPlayingShown];
    }
    if (constraintsNowPlayingHidden) {
        [leftView removeConstraints:constraintsNowPlayingHidden];
    }
    
    auto spacing = IApp::instance()->themeManager()->style().get("playlistpanel nowplayingtitle").padding();
    nowPlayingHeight = 8 + spacing.top + spacing.bottom;
    NowPlayingImageTitle *nowPlayingTitle = [AppDelegate sharedDelegate].nowPlayingImageTitle;
    nowPlayingTitle.yOffsetRelativeToCenter = (int)(spacing.top - spacing.bottom)/2;
    if (nowPlayingHeightConstraint == nil) {
        nowPlayingHeightConstraint = [NSLayoutConstraint constraintWithItem:albumArtHolder attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:albumArtHolder attribute:NSLayoutAttributeWidth multiplier:1 constant:nowPlayingHeight];
        [leftView addConstraint:nowPlayingHeightConstraint];
    } else {
        nowPlayingHeightConstraint.constant = nowPlayingHeight;
    }
    
    constraintsNowPlayingShown = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[albumArtHolder]|" options:0 metrics:nil views:constraintsNowPlayingViews];
    constraintsNowPlayingHidden = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[playlist]-(==nowPlayingHeight)-|" options:0 metrics:@{@"nowPlayingHeight":@(nowPlayingHeight)} views:constraintsNowPlayingViews];
    [leftView addConstraints:shown ? constraintsNowPlayingShown : constraintsNowPlayingHidden];
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext* context) {
        context.duration = 0.4;
        // no animation on lion, but yes animation on mountain lion
        if ([context respondsToSelector:@selector(setAllowsImplicitAnimation:)]) {
            context.allowsImplicitAnimation = YES;
        }
        [leftView layoutSubtreeIfNeeded];
    } completionHandler:nil];
}

- (void)setTopHeight:(int)topHeight
{
    CGRect frame;
    frame = self.topContainer.frame;
    frame.size.height = topHeight;
    frame.origin.y = self.topContainer.superview.frame.size.height - topHeight;
    self.topContainer.frame = frame;
    
    const CGFloat kBottomBorderThickness = 23; // like yosemite finder
    
    NSView *splitView = [AppDelegate sharedDelegate].mainSplitView;
    frame = splitView.frame;
    frame.size.height = self.topContainer.frame.origin.y - frame.origin.y;
    frame.origin.y = kBottomBorderThickness;
    splitView.frame = frame;
    
    [self.mainWindow setContentBorderThickness:kBottomBorderThickness forEdge:NSMinYEdge];
    
    for (NSView *view in [self.topContainer subviews]) {
        frame = [view frame];
        frame.origin.y -= 0;
        view.frame = frame;
    }
    
    CGRect headerRect = musicListView.headerView.frame;
    headerRect.size.height = 20;
    musicListView.headerView.frame = headerRect;
}

- (SongsTableView *)musicListView
{
    return musicListView;
}

- (void)recreateMusicListView
{
    // list view should be recreated otherwise there are background color artifacts...
    
    NSView *parent = self.musicListViewScroll.superview;
    
    CGPoint scrollOffset = self.musicListViewScroll.contentView.bounds.origin;
    [self.musicListViewScroll removeFromSuperview];
    self.musicListViewScroll = [[ScrollViewWorkaround alloc] initWithFrame:[self.musicListViewScroll frame]];
    
    self.musicListViewScroll.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    self.musicListView = [[SongsTableView alloc] initWithFrame:self.musicListView.frame];
    self.musicListViewScroll.documentView = self.musicListView;
    [parent addSubview:self.musicListViewScroll];

    [self.musicListViewScroll setHasHorizontalScroller:YES];
    [self.musicListViewScroll setHasVerticalScroller:YES];
    [self.musicListViewScroll setAutohidesScrollers:YES];
    [self.musicListView setRowHeight:22];
    [self.musicListView setIntercellSpacing:CGSizeMake(3,0)];
    [self.musicListViewScroll setFocusRingType:NSFocusRingTypeNone];
    [self.musicListView setFocusRingType:NSFocusRingTypeNone];
    [self.musicListView setAllowsTypeSelect:YES];
    [self.musicListView setAllowsMultipleSelection:YES];
    [self.musicListView setAllowsEmptySelection:YES];
    
    self.musicListViewScroll.scrollerKnobStyle = [[CocoaThemeManager sharedManager] knobStyle];
    
    // columns
    //[title, identifier, editable, min, default, max]
    NSArray *columns = @[
                         @[@"position",@"#",@NO,@13,@13,@13],
                         @[@"source",@"Src",@NO,@29,@29,@29],
                         @[@"artist",@"Artist",@YES,@40,@125,@1000],
                         @[@"track",@"No",@YES,@40,@40,@100],
                         @[@"title",@"Title",@YES,@40,@125,@1000],
                         @[@"durationMillis",@"Time",@NO,@40,@50,@1000],
                         @[@"album",@"Album",@YES,@40,@125,@1000],
                         @[@"genre",@"Genre",@YES,@40,@70,@1000],
                         @[@"year",@"Year",@YES,@40,@70,@1000],
                         @[@"rating",@"Rating",@NO,@40,@([self calculateRatingColumnWidth]),@1000],
                         @[@"playCount",@"Plays",@NO,@40,@40,@1000],
                         @[@"disc",@"Disc",@YES,@40,@40,@1000],
                         @[@"lastPlayed",@"Last Played",@NO,@40,@90,@1000],
                         @[@"creationDate",@"Added On",@NO,@40,@90,@1000],
                         @[@"albumArtist",@"Album Artist",@YES,@80,@125,@1000]
                            ];
    
    for (NSArray *columnDef in columns) {
        NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier:columnDef[0]];
        column.editable = [columnDef[2] boolValue];
        column.minWidth = [columnDef[3] intValue];
        column.width = [columnDef[4] intValue];
        column.maxWidth = [columnDef[5] intValue];
        if (![columnDef[0] isEqualToString:@"source"]) {
            column.sortDescriptorPrototype = [NSSortDescriptor sortDescriptorWithKey:columnDef[0] ascending:YES];
        }
        
        ThemedHeaderCell *headerCell = [[ThemedHeaderCell alloc]
                                  initTextCell:columnDef[1]];
        [headerCell setIdentifier:columnDef[0]];
        [column setHeaderCell:headerCell];

        [self.musicListView addTableColumn:column];
    }
    [self setRatingColumnWidth];
    
    CGRect frame = musicListView.headerView.frame;
    frame.size.height = 24;
    musicListView.headerView.frame = frame;
    //musicListViewScroll.superview.hidden = YES;
    
    musicListView.usesAlternatingRowBackgroundColors = YES;
    [musicListView setTarget:self];
    [musicListView setAction:@selector(singleClick:)];
    [musicListView setDoubleAction:@selector(doubleClick:)];
    // deliberately different than before... this is because otherwise the xib would destroy the position of the src column
    musicListView.autosaveName = @"songs_table";
    musicListView.autosaveTableColumns = YES;
    
    [self.musicListView awakeFromNib];
    
    [self.musicListView setDelegate:self];
    [self.musicListView setDataSource:self];
    [self.musicListView reloadData];
    
    [self setMusicListContentInsets];
    
    dispatch_async(dispatch_get_main_queue(),^{
        [self setRatingColumnWidth];
        [self.musicListViewScroll.contentView setBoundsOrigin:scrollOffset];
    });
 
    [lineAtBottomOfTopBorder removeFromSuperview];
    if ([[CocoaThemeManager sharedManager] themePrefix] == nil) {
        lineAtBottomOfTopBorder = [[ColoredView alloc] init];
        [lineAtBottomOfTopBorder setBackgroundColor:[NSColor colorWithDeviceRed:149.0f/255.0f green:148.0f/255.0f blue:150.0f/255.0f alpha:1.0f]];
        lineAtBottomOfTopBorder.translatesAutoresizingMaskIntoConstraints = NO;
        [self.mainWindow.contentView addSubview:lineAtBottomOfTopBorder positioned:NSWindowAbove relativeTo:nil];
        NSDictionary *views = NSDictionaryOfVariableBindings(lineAtBottomOfTopBorder);
        [self.mainWindow.contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[lineAtBottomOfTopBorder]|" options:0 metrics:nil views:views]];
        [self.mainWindow.contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-47-[lineAtBottomOfTopBorder(==1)]" options:0 metrics:nil views:views]];
    }
}

- (void)setMusicListView:(SongsTableView *)aMusicListView
{
    musicListView = aMusicListView;
}
    
- (NSString *)artistAlbum:(NSManagedObject *)song
{
    NSString *artist = [song valueForKey:@"artist"];
    NSString *album = [song valueForKey:@"album"];

    if ([artist length] == 0) {
        return album;
    }
    if ([album length] == 0) {
        return artist;
    }
    return [NSString stringWithFormat:@"%@ - %@", artist, album];
}

- (void)setDisplayedArtist:(NSString *)artist album:(NSString *)album song:(NSString *)song
{
    if (song == nil) {
        song = @"";
    }
    if (album == nil) {
        album = @"";
    }
    if (artist == nil) {
        artist = @"";
    }

    NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
    paragraphStyle.alignment = NSLeftTextAlignment;
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    
    NSColor *textColor = [[CocoaThemeManager sharedManager] textColor];
    NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
    if (prefix == nil) {
        textColor = [NSColor colorWithDeviceWhite:36.0f/255.0f alpha:1.0f];
    }
    
    NSShadow *shadow = [[CocoaThemeManager sharedManager] categoryShadow];

    BOOL dark = [[CocoaThemeManager sharedManager] themePrefix] != nil;
    NSFont *titleFont = dark ? [NSFont fontWithName:@"HelveticaNeue-Medium" size:13.0f] : Writer::systemFont(13, false);
    NSMutableDictionary *attrTitle = [@{NSForegroundColorAttributeName:textColor,
                                      NSParagraphStyleAttributeName:paragraphStyle,
                                        NSFontAttributeName:titleFont} mutableCopy];
    if (shadow) {
        [attrTitle setObject:shadow forKey:NSShadowAttributeName];
    }
    [songTitle setAttributedStringValue:[[NSAttributedString alloc] initWithString:song
                        attributes:attrTitle]];
        
    NSString *albumArtist;
    if ([artist length] == 0) {
        albumArtist = album;
    } else if ([album length] == 0) {
        albumArtist = artist;
    } else {
        albumArtist = [NSString stringWithFormat:@"%@ - %@", artist, album];
    }
    
    auto theme = IApp::instance()->themeManager()->current();
    NSMutableAttributedString *attrAlbumArtist = [[NSMutableAttributedString alloc] init];
    
    NSFont *albumArtistFont = dark ? [NSFont fontWithName:@"HelveticaNeue-Medium" size:11.0f] : Writer::systemFont(11, false);
    [attrAlbumArtist appendAttributedString:[[NSAttributedString alloc] initWithString:artist attributes:@{NSForegroundColorAttributeName:textColor,
           NSFontAttributeName:albumArtistFont}]];
    [attrAlbumArtist appendAttributedString:[[NSAttributedString alloc] initWithString:@" "]];
    
    textColor = [[[CocoaThemeManager sharedManager] textColor] colorWithAlphaComponent:theme->miniplayerSubSubtitleText().color().alpha()];
    if (prefix == nil) {
        // was 120
        textColor = [NSColor colorWithDeviceWhite:112.0f/255.0f alpha:1.0f];
        //textColor = [[CocoaThemeManager sharedManager] buttonContentColorLight];
    }
    
    [attrAlbumArtist appendAttributedString:[[NSAttributedString alloc] initWithString:album attributes:@{NSForegroundColorAttributeName:textColor,
          NSFontAttributeName:albumArtistFont}]];
    [attrAlbumArtist addAttribute:NSParagraphStyleAttributeName value:paragraphStyle range:NSMakeRange(0, attrAlbumArtist.length)];
    if (shadow) {
        [attrAlbumArtist addAttribute:NSShadowAttributeName value:shadow range:NSMakeRange(0, attrAlbumArtist.length)];
    }
    
    [songAlbumArtist setAttributedStringValue:attrAlbumArtist];
    
    [songTitle invalidateIntrinsicContentSize];
    [songAlbumArtist invalidateIntrinsicContentSize];
    
    [self resizeTopViews];
    
    [statusBarManager setDisplayedAlbumArtist:albumArtist song:song artist:artist];
}

- (void)setDisplayedInfoForSong:(shared_ptr<ISong>)song
{
    NSString *artist = @"";
    NSString *album = @"";
    NSString *title = @"";
    
    if (song) {
        artist = convertString(song->artist());
        album = convertString(song->album());
        title = convertString(song->title());
    }
    
    [self setDisplayedArtist:artist album:album song:title];
    NSUserDefaultsController *userDefaults = [NSUserDefaultsController sharedUserDefaultsController];
    if (mountainLionOrBetter() && [[[userDefaults values] valueForKey:kNotificationCenterEnabled] boolValue]) {
        
        [self sendToNotificationCenter:song];
    }
    [self displayAlbumArtForSong:song];
    
    // nicecast
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
#ifdef ENABLE_PADDLE
    NSURL *niceUrl = [[[[[[fileManager URLForDirectory:NSLibraryDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil] URLByAppendingPathComponent:@"Containers"] URLByAppendingPathComponent:@"com.treasurebox.gear"] URLByAppendingPathComponent:@"Data"] URLByAppendingPathComponent:@"Library"] URLByAppendingPathComponent:@"Application Support"];
#else
    NSURL *niceUrl = [fileManager URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
#endif
    niceUrl = [niceUrl URLByAppendingPathComponent:@"Nicecast" isDirectory:YES];
    [fileManager createDirectoryAtPath:[niceUrl path] withIntermediateDirectories:YES attributes:nil error:nil];
    niceUrl = [niceUrl URLByAppendingPathComponent:@"NowPlaying.txt" isDirectory:NO];
    NSOutputStream *niceStream = [[NSOutputStream alloc] initWithURL:niceUrl append:NO];
    [niceStream open];
    void (^appendString)(NSString *) = ^(NSString *string){
        NSData *data = [string dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:YES];
        [niceStream write:(const uint8_t *)[data bytes] maxLength:[data length]];
    };
    appendString([NSString stringWithFormat:@"Title: %@\n",title != nil ? title : @""]);
    appendString([NSString stringWithFormat:@"Artist: %@\n",artist != nil ? artist : @""]);
    appendString([NSString stringWithFormat:@"Album: %@\n",album != nil ?  album : @""]);
    [niceStream close];
}

- (void)applyTheme
{
    NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
    [[AppDelegate sharedDelegate].mainSplitView applyTheme:translucencyOffset];
    
    NSArray *texts = @[self.songTitle, self.songAlbumArtist, self.songRemaining, self.songElapsed, self.songCountLabel];
    for (NSTextField *textField in texts) {
        [textField setTextColor:[[CocoaThemeManager sharedManager] textColor]];
    }
  
  
    
    [self.songAlbumArtist setFont:Writer::systemFont(11.0f, false)];
    
    NSFont *commonFont = Gui::Writer::convertFont(IApp::instance()->themeManager()->current()->miniplayerTimeText().font());
    if (prefix != nil) {
        commonFont = [NSFont fontWithName:@"HelveticaNeue-Medium" size:[[[commonFont fontDescriptor] objectForKey:NSFontSizeAttribute] floatValue] + 1];
    }

    [self.songRemaining setFont:commonFont];
    [self.songElapsed setFont:commonFont];
    
    NSColor *commonColor = [[CocoaThemeManager sharedManager] buttonContentColorLight];
    if ([[CocoaThemeManager sharedManager] themePrefix] == nil) {
        commonColor = [NSColor colorWithDeviceWhite:36.0f/255.0f alpha:1.0f];
    }
    [self.songRemaining setTextColor:commonColor];
    [self.songElapsed setTextColor:commonColor];
    [self.songCountLabel setTextColor:commonColor];
    
    
    [[AppDelegate sharedDelegate] progressIndicator].color = [[CocoaThemeManager sharedManager] buttonContentColor];
    
    // for some strange reason this does not work in IB
    [shuffleButton setImage:[[CocoaThemeManager sharedManager] shuffleIcon:NO]];
    [repeatButton setImage:[[CocoaThemeManager sharedManager] repeatIcon:NO]];
    [shuffleButton setAlternateImage:[[CocoaThemeManager sharedManager] shuffleIcon:YES]];
    [repeatButton setAlternateImage:[[CocoaThemeManager sharedManager] repeatIcon:YES]];
    
    
    // this caused problems in retina/moderndark...
    //NSImage *outputsImage = [[NSImage imageNamed:@"outputs"] imageTintedWithColor:[NSColor colorWithDeviceWhite:48.0f/255.0f alpha:1.0f]];
    [shuffleButton.cell setImageScaling:NSImageScaleNone];
    [repeatButton.cell setImageScaling:NSImageScaleNone];
    [self.outputsButton.cell setImageScaling:NSImageScaleNone];
    //[outputsImage imageTintedWithColor:[NSColor whiteColor]];
    //NSLog(@"shuffle: %@ repeat: %@", NSStringFromRect(shuffleButton.frame), NSStringFromRect(repeatButton.frame));
    
    if (prefix == nil) {
        [self.outputsButton setImage:[NSImage imageNamed:@"outputs"]];
        [self.outputsButton setAlternateImage:[NSImage imageNamed:@"outputs-blue"]];
    } else {
        [self.outputsButton setImage:[NSImage imageNamed:@"outputs-white"]];
        [self.outputsButton setAlternateImage:[NSImage imageNamed:@"outputs-white"]];
    }   
        
    //[shuffleButton setImage:[NSImage imageNamed:@"shufflesmall" tintWithColor:[[CocoaThemeManager sharedManager] buttonContentColor]]];
    //[repeatButton setImage:[NSImage imageNamed:@"repeatsmall" tintWithColor:[[CocoaThemeManager sharedManager] buttonContentColor]]];
    [self setShareImage];
    
    [self.searchField setTextColor:[[CocoaThemeManager sharedManager] buttonContentColorLight]];
    [self.searchFieldCustom setTextColor:[[CocoaThemeManager sharedManager] buttonContentColorLight]];
 
    self.volumeIconMax.imageScaling = NSImageScaleNone;
    self.volumeIcon.imageScaling = NSImageScaleNone;
    

    if (prefix == nil) {
        self.volumeIconMax.image = [NSImage imageNamed:@"volume"];
        self.volumeIcon.image = [NSImage imageNamed:@"volume-zero"];
    } else {
        self.volumeIconMax.image = [NSImage imageNamed:@"volume-moderndark"];
        self.volumeIcon.image = [NSImage imageNamed:@"volume-zero-moderndark"];
    }
    
    // simply put, this is voodoo:
    [self.playlistManager applyTheme];
    dispatch_async(dispatch_get_main_queue(),^{
        [self.playlistManager applyTheme];
    });
    
    [self.albumViewController applyTheme];
    
    if ([albumArtColorView respondsToSelector:@selector(setBackgroundColor:)]) {
        NSColor *color = [[CocoaThemeManager sharedManager] playlistsBackgroundColor];
        if (yosemiteOrBetter() && !vibrancyEnabled() && [[CocoaThemeManager sharedManager] themePrefix] == nil) {
            // special vibrancy disabled mode
        } else {
            if ([color alphaComponent] > 0) {
                color = [color shadowWithLevel:0.1];
            }
        }
        [(id)albumArtColorView setBackgroundColor:color];
    }
    
    if (lionOrBetter()) {
        
        NSColor *color = [[CocoaThemeManager sharedManager] fullScreenButtonColor];
        if (color != nil) {
            NSButton *fullScreenButton = [self.mainWindow standardWindowButton:NSWindowFullScreenButton];
            [fullScreenButton setImage:[[fullScreenButton image] imageTintedWithColor:color]];
        }
    }
    
    [self showBigText];
    
    
    [self setDisplayedInfoForSong:IApp::instance()->player()->songEntryConnector().value().song()];
    
    // this is needed for retheming table without visual glitches:
    [self.musicListView reloadData];
    
    [self setupSharing];
    
    [self updatePlayImage];
    
    // this will force a deeper redraw, necessary because of layer-backing
    self.musicListView.rowHeight = self.musicListView.rowHeight;
    
    if ([[CocoaThemeManager sharedManager] searchBackground] == nil) {
        [self.searchField setHidden:NO];
        [self.searchFieldCustom setHidden:YES];
    } else {
        [self.searchField setHidden:YES];
        [self.searchFieldCustom setHidden:NO];
    }
    
    // update layout height
    [self setConstraintsForNowPlayingShown:nowPlayingShown];
    
    [self recreateMusicListView];
    
    [songCountLabel setFont:prefix == nil ? [NSFont systemFontOfSize:11] : [NSFont fontWithName:@"HelveticaNeue-Medium" size:11]];
    CGRect frame = songCountLabel.frame;
    frame.origin.y = prefix == nil ? 2 : 5;
    songCountLabel.frame = frame;
    
    [self.mainWindow.contentView setNeedsDisplayRecursive];
}

- (NSArray *)topMostItems
{
    return @[self.playButton, self.prevButton, self.nextButton, self.songTitle, self.searchField, self.searchFieldCustom, self.volumeHolder, self.shareButton, self.songElapsed, self.songRemaining, self.songAlbumArtist,
	self.playbackPositionSlider];
}

static CGFloat kTopAllOffset = -17.0f;
static CGFloat kTopMostFullscreenOffset = 17.0f + kTopAllOffset;
static CGFloat kTopContainerFullScreenOffset = -5.0f + kTopAllOffset;

static CGFloat kSongInfoFullscreenOffset = 0.0f;

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
    //[self setTopHeight:kTopHeightFullScreen];
    
    if (!fullscreened) {
        fullscreened = YES;
        for (NSView *view in [self topMostItems]) {
            CGRect frame = view.frame;
            frame.origin.y += kTopMostFullscreenOffset;
            view.frame = frame;
        }
        
        CGRect frame = topContainer.frame;
        frame.origin.y += kTopContainerFullScreenOffset;
        topContainer.frame = frame;
        
        /*frame = self.shareButton.frame;
        frame.origin.y += kTopContainerFullScreenOffset;
        self.shareButton.frame = frame;*/
        
        frame = self.songTitle.frame;
        frame.origin.y += kSongInfoFullscreenOffset;
        self.songTitle.frame = frame;
        frame = self.songAlbumArtist.frame;
        frame.origin.y += kSongInfoFullscreenOffset;
        self.songAlbumArtist.frame = frame;
        
        self.mainWindow.styleMask &= ~(NSMiniaturizableWindowMask | NSClosableWindowMask);
    }
    
    [self resizeTopViews];
    
    [AppDelegate showInDock];
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"FullScreen"];
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
    if (fullscreened) {
        self.mainWindow.styleMask |= NSMiniaturizableWindowMask | NSClosableWindowMask;
        
        fullscreened = NO;
        for (NSView *view in [self topMostItems]) {
            CGRect frame = view.frame;
            frame.origin.y -= kTopMostFullscreenOffset;
            view.frame = frame;
        }
        
        CGRect frame = topContainer.frame;
        frame.origin.y -= kTopContainerFullScreenOffset;
        topContainer.frame = frame;
        
        /*frame = self.shareButton.frame;
        frame.origin.y -= kTopContainerFullScreenOffset;
        self.shareButton.frame = frame;*/
        
        frame = self.songTitle.frame;
        frame.origin.y -= kSongInfoFullscreenOffset;
        self.songTitle.frame = frame;
        frame = self.songAlbumArtist.frame;
        frame.origin.y -= kSongInfoFullscreenOffset;
        self.songAlbumArtist.frame = frame;
    }
    
    [self resizeTopViews];
    
    [[NSUserDefaults standardUserDefaults] setBool:NO forKey:@"FullScreen"];
    
    //[self setTopHeight:kTopHeight];
}

- (shared_ptr<PlaybackController>)playbackController
{
    return PlaybackController::instance();
}

- (void)viewDidLoad
{
	for (NSView *view in [self topMostItems]) {
		view.frame = NSOffsetRect(view.frame, 0, -20);
	}

/*
    for (NSView *item in [self topMostItems]) {
//        NSView *destination = [self.mainWindow.contentView superview];
        NSView *destination = self.mainWindow.contentView;

        NSRect newFrame = [destination convertRect:item.frame fromView:[item superview]];
        [item removeFromSuperview];
        [destination addSubview:item];
        [item setFrame:newFrame];
    }
*/
    
    [self applyTheme];
    
    [self.musicListView registerForDraggedTypes:@[kPasteBoardTypeSong]];
    
    [self.albumSplitView setPositionOfDivider:0 animated:NO];
    
    [self updateRepeatMixPossible:nil];
    
    [[NSUserDefaultsController sharedUserDefaultsController] addObserver:self forKeyPath:@"values.sortDescriptors" options:NSKeyValueObservingOptionInitial context:nil];
    [[NSUserDefaultsController sharedUserDefaultsController] addObserver:self forKeyPath:@"values.ForceSortByAlbumName" options:NSKeyValueObservingOptionNew context:nil];
    [[NSUserDefaultsController sharedUserDefaultsController] addObserver:self forKeyPath:@"values.FiveStarRatings" options:NSKeyValueObservingOptionNew context:nil];
    
    selectedPlaylistConnection = IApp::instance()->selectedPlaylistConnector().connect([self](const std::pair<PlaylistCategory,shared_ptr<IPlaylist>> &p) {
#if DEBUG
        std::cout << "selected playlist: " << p.second->playlistId() << std::endl;
#endif
        auto &playlist = p.second;
        
        if (playlist) {
            auto predicate = playlist->filterPredicate();
#if DEBUG
            std::cout << "predicate: " << predicate.value() << std::endl;
#endif
            
            [self selectSearchKey:predicate.key()];
            [self.searchField setStringValue:convertString(predicate.value())];
            [self.searchFieldCustom setStringValue:convertString(predicate.value())];
            
            ignoreSearch = YES;
            // feed the value back
            [self search:self.searchField];
            ignoreSearch = NO;
            
            auto sortKey = playlist->sortKeyAndAscending();
            [self.musicListView setSortDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:convertString(sortKey.first) ascending:sortKey.second]]];
        
            [naviBarView setInfo:NaviBarInfo::forPlaylist(playlist)];
        }
    });
    
    [self displayAlbumArtForSong:nil];
    [self updatePlayImage];
    [self setPlaying:NO];
    
    
    [self setDisplayedInfoForSong:nil];
    
    //[[songAlbumArtist cell] setBackgroundStyle:NSBackgroundStyleRaised];
    
    [playButton addObserver:self forKeyPath:@"intValue" options:NSKeyValueObservingOptionNew context:nil];
    
    
    statusBarManager = [[StatusBarManager alloc] init];
    statusBarManager.delegate = self;
    [statusBarManager setupStatusBar];
    
    [self.shareButton setEnabled:NO];
    [self setupSharing];
    
    // this would fix sort order between versions,
    // but is unnecessary...
    //[musicListView setSortDescriptors:[musicListView sortDescriptors]];
    
    // ...because:
    NSString *kKeyFirst = @"EverUsedVersionWithPositionBasedOrder";
    if (![[NSUserDefaults standardUserDefaults] boolForKey:kKeyFirst]) {
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:kKeyFirst];
        [musicListView setSortDescriptors:[NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"position" ascending:YES]]];
    }
    
    //id clipView = [[self.musicListView enclosingScrollView] contentView];
    
    // this would do the job, but inhibits a serious warning message...
    /*[[NSNotificationCenter defaultCenter] addObserverForName:NSViewBoundsDidChangeNotification object:clipView queue:nil usingBlock:^(NSNotification *note) {
        
        lastScroll = [NSDate timeIntervalSinceReferenceDate];
    }];*/
    [[NSNotificationCenter defaultCenter] addObserverForName:kNotificationScrollByUser object:[self.musicListView enclosingScrollView] queue:nil usingBlock:^(NSNotification *note) {
        lastScroll = [NSDate timeIntervalSinceReferenceDate];
    }];

    // remember to set this to YES instead of deleting line, otherwise preview versions could be broken
    BOOL instantHidden = NO;
#ifdef HIDE_CURRENTINSTANT_COLUMN
    instantHidden = YES;
#endif
    [[musicListView tableColumnWithIdentifier:@"currentInstant"] setHidden:instantHidden];
    
    
    //[self setupToggleColumnMenu];
    
    [[NSUserDefaultsController sharedUserDefaultsController] addObserver:self forKeyPath:[NSString stringWithFormat:@"values.%@", kUserDefaultsShowVolume] options:NSKeyValueObservingOptionInitial context:nil];
    [[NSUserDefaultsController sharedUserDefaultsController] addObserver:self forKeyPath:[NSString stringWithFormat:@"values.%@", kUserDefaultsShowShareButton] options:NSKeyValueObservingOptionInitial context:nil];

    auto player = IApp::instance()->player();
    playedSongConnection = player->songEntryConnector().connect([self](const SongEntry &entry){
        [self setDisplayedInfoForSong:entry.song()];
        
        bool present = (bool)entry;
        [prevButton setEnabled:present];
        // setEnabled: simply just does not work
        [self.prevMenuitem setAction:present ? @selector(prev:) : nil];
        [nextButton setEnabled:present];
        [self.nextMenuitem setAction:present ? @selector(next:) : nil];
        [self.shareButton setEnabled:present];
        
        [self reloadTable];
        
        [self scrollToSongIfNecessary];
    });
    playingConnection = player->playingConnector().connect([self](const bool playing){
       
        [self setPlaying:playing];
        [self updatePlayImage];
    });
    
    shuffleConnection = player->shuffle().connector().connect([self](const bool shuffle){
        [self.shuffleButton setIntValue:shuffle];
    });
    
    repeatConnection = player->repeat().connector().connect([self](const IPlayer::Repeat repeat){
        [self.repeatButton setIntValue:static_cast<int>(repeat)];
    });
    
    elapsedConnection = player->elapsedTimeConnector().connect([self](const string &str){
        self.songElapsed.stringValue = convertString(str);
    });
    
    remainingConnection = player->remainingTimeConnector().connect([self](const string &str){
        self.songRemaining.stringValue = convertString(str);
    });
    
    ratioConnection = player->songRatioConnector().connect([self](const float ratio){
        [self setRatio:ratio];
    });
}

- (void)updateRepeatMixPossible:(NSNotification *)note
{
    [self.repeatButton setAllowsMixedState:[[NSUserDefaults standardUserDefaults] boolForKey:kSingleSongLoopPossible]];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (object == [NSUserDefaultsController sharedUserDefaultsController]) {
         
        if ([keyPath hasSuffix:kUserDefaultsShowVolume]) {
        
            BOOL value = [[NSUserDefaults standardUserDefaults] boolForKey:kUserDefaultsShowVolume];

            [self.volumeHolder setHidden:!value];
            [self resizeTopViews];
        } else if ([keyPath hasSuffix:kUserDefaultsShowShareButton]) {
            BOOL value = [[NSUserDefaults standardUserDefaults] boolForKey:kUserDefaultsShowShareButton];
            if (!mountainLionOrBetter()) {
                value = NO;
            }
            [self.shareButton setHidden:!value];
            [self setupSharing];
        } else if ([keyPath isEqualTo:@"values.sortDescriptors"]) {
            [self sortChanged:nil];
        } else if ([keyPath isEqualTo:@"values.ForceSortByAlbumName"]) {
            [self.musicListView resetSortDescriptors];
        } else if ([keyPath isEqualTo:@"values.FiveStarRatings"]) {
            dispatch_async(dispatch_get_main_queue(),^{
                [self setRatingColumnWidth];
            });
        //    [self reloadTable];
        }
    } else {
        //[self observeValueForKeyPath:keyPath ofSong:object context:context];
    }
}

- (CGFloat)calculateRatingColumnWidth
{
    BOOL value = [[NSUserDefaults standardUserDefaults] boolForKey:@"FiveStarRatings"];
    return value ? 100.0f : 44.0f;
}

- (void)setRatingColumnWidth
{
    NSTableColumn *column = [self.musicListView tableColumnWithIdentifier:@"rating"];
    [column setWidth:[self calculateRatingColumnWidth]];
}

- (void)updatePlayImage
{
    bool playing = IApp::instance()->player()->playingConnector().value();
    
    if ([playButton intValue] != playing) {
        [playButton setIntValue:playing];
    }
    [self updatePlayCenter];

    [statusBarManager setPlaying:playing];
    
    static NSMapTable *views = nil;
    if (!views) {
        views = [NSMapTable mapTableWithStrongToStrongObjects];
        auto add = [](NSView *view){
            [views setObject:@(view.superview.frame.size.height - (view.frame.origin.y + view.frame.size.height)) forKey:view];
        };
        
        add(self.playButton);
        add(self.nextButton);
        add(self.prevButton);
    }
    
    auto style = IApp::instance()->themeManager()->current()->style();
    auto setImages = [&](const string &controlName, NSButton *control){
        auto variant = [&](const string &postfix){
            return style.get(string("controls ") + controlName + " " + postfix);
        };
        
        [control setImage:[NSImage imageNamed:convertString(variant("normal").imageName())]];
        [control setAlternateImage:[NSImage imageNamed:convertString(variant("pushed").imageName())]];
        [control setButtonType:NSMomentaryChangeButton];
        
        NSImage *bezelImage = [NSImage imageNamed:convertString(variant("bezel").imageName())];
        //[control.cell setBezelImage:bezelImage];
        //[control.cell setBezelAlternateImage:[NSImage imageNamed:convertString(variant("bezel_pushed").imageName())]];
        
        [[control cell] setXOffset:variant("normal").padding().left - variant("normal").padding().right];
        
        auto margin = style.get(string("controls ") + controlName).margin();
        marginConstraints[control].first.constant = margin.left;
        marginConstraints[control].second.constant = margin.right;
    };
    setImages(playing ? "pause" : "play", playButton);
    setImages("prev", prevButton);
    setImages("next", nextButton);
    
    [playButton.superview layoutSubtreeIfNeeded];
}

- (void)setPlaying:(BOOL)value
{
    [self.playMenuitem setTitle:value ? @"Pause" : @"Play"];
    
    /*
    if (value == NO) {
        [self setDisplayedInfoForSong:nil];
        [[PlaybackController sharedController] playSong:nil withPlaybackData:shared_ptr<PlaybackData>()];
    }
    [self reloadVisibleSongs];*/
}

- (void)playSong:(SongEntry)aSongToPlay
{
    NSAssert([NSThread isMainThread], @"playsong must be called from main thread!");
 
#ifdef ENABLE_PADDLE
    if (![[Paddle sharedInstance] productActivated] && [[[Paddle sharedInstance] daysRemainingOnTrial] integerValue] == 0) {
        return;
    }
#endif
    
    IApp::instance()->player()->play(aSongToPlay);
    
    // scroll if necessary (this depends on playbackcontroller song being set)
    if (![self userMoved] && !skipJumpNextTime) {
        [self jumpToSong];
    }
    if (skipJumpNextTime) {
        skipJumpNextTime = NO;
    }
}

- (BOOL)userMoved
{
    return userInitiatedScrollOrigin != [self currentScrollOrigin] || IApp::instance()->player()->playlistCurrentlyPlaying() != IApp::instance()->selectedPlaylist();
}

- (void)pauseSong
{
    auto player = IApp::instance()->player();
    if (player->songEntryConnector().value() && player->playingConnector().value()) {
        player->play();
    }
}

- (void)pauseSong:(NSNotification *)notification
{
    [self pauseSong];
}

- (void)screenLocked:(NSNotification *)notification
{
    //NSLog(@"distributed noti: %@", [notification name]);

    if ([[NSUserDefaults standardUserDefaults] boolForKey:@"PausePlaybackOnLock"]) {
        [self pauseSong:notification];
    }
}

- (void)userInitiatedPlaySong:(SongEntry)songToPlay
{
    visibleEntries->setNoRepeatBase(songToPlay);
    skipJumpNextTime = YES;
 
    [self playSong:songToPlay];
    if (visibleEntries == IApp::instance()->queueSongArray()) {
        return;
    }
    // need to jump e.g. when playing 'first' in shuffle mode
    [self jumpToSong:nil];
    
    userInitiatedScrollOrigin = [self currentScrollOrigin];
}

- (void)setRatio:(float)ratio
{
    [playbackPositionSlider setDoubleValue:ratio * [playbackPositionSlider maxValue]];
}

- (IBAction)positionChange:(id)sender
{
    double value = [playbackPositionSlider doubleValue] / [playbackPositionSlider maxValue];
    static double lastValue = -1;
    static float lastTime = 0;
 
    float now = [NSDate timeIntervalSinceReferenceDate];
    float timeDiff = now - lastTime;
    
    // we receive two events on click: mouse down and up. we may stil want to go back to same time point multiple times
    if (value != lastValue || timeDiff >= 0.1f) {
        IApp::instance()->player()->setRatio(value);
        lastValue = value;
    }
}

- (IBAction)volumeChange:(id)sender
{
    PlaybackController::instance()->setVolume([sender doubleValue] / [sender maxValue]);

    static BOOL changingVolume = NO;
    if (!changingVolume) {
        changingVolume = YES;
        NSTimeInterval delayInSeconds = 1.0f;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
            
            changingVolume = NO;
            [[[NSUserDefaultsController sharedUserDefaultsController] values] setValue:@([sender doubleValue]) forKey:@"volume"];
        });
    }
}

- (BOOL)browsingMode
{
    /*    id<PlaylistProtocol> playlist = [AppDelegate sharedDelegate].selectedPlaylist;
     if ([playlist isKindOfClass:[BrowserPlaylist class]]) {
     */
    return NO;
}

- (void)adjustCollection
{
    auto app = IApp::instance();
    if (!app) {
        return;
    }
    auto playlist = app->selectedPlaylist();
    static weak_ptr<IPlaylist> lastValue;
    BOOL change = (lastValue.lock() != playlist);
    lastValue = playlist;
    
    if (playlist && playlist->groupingsEnabled()) {
        self.albumViewVisible = YES;
        
        if (change) {
            // type select made possible
            [self.imageBrowserView.window makeFirstResponder:self.imageBrowserView];
        }
        
        // no more three panel view
        if (YES || !playlist->selectedGroupingConnector().value()) {
            
            [self.albumSplitView setPositionOfDivider:self.albumSplitView.frame.size.width animated:YES];
            self.albumViewController.browserPlaylist = playlist;
            // might not need reload here!
            //[self.albumViewController reload];
        } else {
            CGFloat cellSize = [[[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKey:kDefaultsKeyAlbumCellSize] floatValue];
            
            [self.albumSplitView setPositionOfDivider:cellSize + 30 animated:YES];
            self.albumViewController.browserPlaylist = playlist;
        }
    } else {
        
        if (change) {
            [self.musicListView.window makeFirstResponder:self.musicListView];
        }
            
        self.albumViewVisible = NO;
        [self.albumSplitView setPositionOfDivider:0 animated:YES];
    }
}

- (IBAction)focusSearch:(id)sender
{
    [searchField becomeFirstResponder];
}

- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)subview
{
 /*   id<PlaylistProtocol> playlist = [AppDelegate sharedDelegate].selectedPlaylist;
    if ([playlist isKindOfClass:[BrowserPlaylist class]]) {
        
        BrowserPlaylist *browserPlaylist = (BrowserPlaylist *)playlist;
        // TODO: only if no song is selected
        if (browserPlaylist.showSelected) {
            return subview == musicListViewScroll;
        } else {
            return subview != musicListViewScroll;
        }
    }*/
    return YES;
}

- (IBAction)search:(NSSearchField *)sender
{
/*
    // this didn't help solving the searchfield sluggishness problem
 
    [searchTimer invalidate];
    searchTimer = [NSTimer scheduledTimerWithTimeInterval:0.5f target:self selector:@selector(doSearch:) userInfo:nil repeats:NO];
}

- (void)doSearch:(NSSearchField *)sender
{
    sender = self.searchField;
    */
    // this is necessary to be able to sort fast
    [self.musicListView scrollRowToVisible:0];
    
    playFirstPending = NO;
    NSString *filter = [sender stringValue];
    if (!filter) {
        filter = self.searchField.stringValue;
    }
#ifdef DEBUG
    //NSLog(@"filter search: %@", filter);
#endif
    self.searchField.stringValue = filter;
    self.searchFieldCustom.stringValue = filter;
    
    //self.albumViewController.filterPredicate = filterFromString(filter, searchCategory);
    
    // if we are on a browserplaylist, we can reset userfilter, since we don't want to filter right panel
    
    shared_ptr<IPlaylist> playlist = App::instance()->selectedPlaylist();
    
    /*    if ([playlist isKindOfClass:[BrowserPlaylist class]]) {
     filter = @"";
     }
     */
    if (!playlist) {
        return;
    }
    
    shared_ptr<ISongArray> desiredArray = playlist->songArray();
    
#if DEBUG
    //std::cout << "selected count: " << desiredArray->size() << std::endl;
#endif
    if (!equals(visibleEntries, desiredArray)) {
        [self setContent:desiredArray];
    }
    
    [self filterChanged:filter category:searchCategory];
    
    [self adjustCollection];
    
    IApp::instance()->recalculateQueue();
}

- (NSArray *)onlyObjects:(NSArray *)objects ofClass:(Class)aClass
{
    return [objects filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary *bindings) {
        
        return [evaluatedObject isKindOfClass:aClass];
    }]];
}

- (void)scrollToSongIfNecessary
{
    auto player = IApp::instance()->player();
    
    auto playlist = player->playlistCurrentlyPlaying();
    if (playlist == IApp::instance()->selectedPlaylist() && [NSDate timeIntervalSinceReferenceDate] >= lastScroll + 25.0f) {
        
        [self jumpToSong];
    }
}

- (void)filterChanged:(NSString *)filter category:(NSString *)categ
{
#ifdef DEBUG
    //NSLog(@"filter changed: %@", filter);
#endif
    if (!ignoreSearch) {
        
        SongPredicate predicate(convertString(categ), convertString(filter), SongPredicate::Contains());
        
        IApp::instance()->selectedPlaylist()->setFilterPredicate(predicate);
    }
#warning TODO: filtering should work
    //[self reloadTable];
    
    [self setupToggleColumnMenu];
}

- (void)showCount:(size_t)count
{
    if (count == 0) {
        
        [self.bigMessageView setHidden:NO];
        [self showBigText];
        
    } else {
        [self.bigMessageView setHidden:YES];
    }
    NSShadow *shadow = [[CocoaThemeManager sharedManager] categoryShadow];
    NSString *countString = [[[ArrayCountTransformer alloc] init] transformedInt:count];
    if (shadow) {
        NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
        paragraphStyle.alignment = NSCenterTextAlignment;
        
        [songCountLabel setAttributedStringValue:[[NSAttributedString alloc] initWithString:countString attributes:@{NSShadowAttributeName:shadow, NSParagraphStyleAttributeName:paragraphStyle}]];
    } else {
        [songCountLabel setStringValue:countString];
    }
}


- (void)centerBigMessage
{
    CGRect frame = self.bigMessageView.frame;
    frame.size.width = self.musicListViewScroll.frame.size.width;
    frame.origin.y = (int)((self.musicListViewScroll.frame.size.height - frame.size.height)/2);
    self.bigMessageView.frame = frame;
}

- (NSAttributedString *)fade:(NSString *)text
{
    text = [text stringByAppendingString:@"  "];
    
    NSMutableAttributedString *attrString = [[NSMutableAttributedString alloc] initWithString:text];
    NSUInteger len = [attrString length];
    NSColor *baseColor = [[CocoaThemeManager sharedManager] bigTextColor];
    for (int i = 0 ; i < len ; i++) {
        
        float ratio = i/((float)len+5);
        
        [attrString setAttributes:@{NSForegroundColorAttributeName:[baseColor colorWithAlphaComponent:(1-ratio)]} range:NSMakeRange(i,1)];
    }
    [attrString appendAttributedString:[[NSAttributedString alloc] initWithString:@"\n"]];
    return attrString;
}

- (void)showBigText
{
    NSString *text = @"";
    if (visibleEntries) {
        text = convertString(visibleEntries->emptyText());
    }
    
    // this can't be done in IB
    [self.musicListViewScroll addSubview:self.bigMessageView];
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // this creates problems:
        /*self.bigMessageView.translatesAutoresizingMaskIntoConstraints = NO;
        [self.bigMessageView.superview addConstraint:[NSLayoutConstraint constraintWithItem:self.bigMessageView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self.bigMessageView.superview attribute:NSLayoutAttributeCenterX multiplier:1 constant:0]];
        [self.bigMessageView.superview addConstraint:[NSLayoutConstraint constraintWithItem:self.bigMessageView attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self.bigMessageView.superview attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];*/
    });
    
    NSArray *lines = [text componentsSeparatedByCharactersInSet: [NSCharacterSet newlineCharacterSet]];
    
    NSArray *fadedLines = [lines map:_([self fade:_])];
    NSMutableAttributedString *attrString = [[NSMutableAttributedString alloc] init];
    
    for (NSAttributedString *line in fadedLines) {
        [attrString appendAttributedString:line];
    }

    [self.bigMessageView setAttributedStringValue:attrString];
    
    [self.bigMessageView sizeToFit];
    // sizeToFit leads to incorrect size
    CGRect frame = self.bigMessageView.frame;
    self.bigMessageView.frame = frame;
    [self centerBigMessage];
}

- (void)reloadTable
{
    unsigned long count = 0;
    if (visibleEntries) {
        _visibleSongs = visibleEntries->songs([self](const SongView &view,SongView::Event event, size_t offset, size_t size){
            
            if (!(view == self->_visibleSongs)) {
                return;
            }
            
            if (event == SongView::Event::Inserted) {
                NSScrollView* scrollView = [self.musicListView enclosingScrollView];
                CGRect visibleRect = scrollView.contentView.visibleRect;
                NSRange range = [self.musicListView rowsInRect:visibleRect];

                if (range.location == NSNotFound || range.location == 0 || range.length == 0) {
                    [self.musicListView reloadData];
                } else {
                    auto center = range.location + range.length/2;
                    [self.musicListView reloadData];
                    if (offset < center) {
                        // lost before branch 0c32373
                        //[self jumpToSongIndex:center+size animate:NO userInitiated:NO];
                    }
                }
            } else {
                [self.musicListView reloadData];
            }
            
            // these would be very slow
            /*
            NSIndexSet *rows = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(offset, size)];
            switch(event) {
                case SongView::Event::Inserted:
                    [self.musicListView beginUpdates];
                    [self.musicListView insertRowsAtIndexes:rows withAnimation:NSTableViewAnimationEffectNone];
                    [self.musicListView endUpdates];
                    break;
                case SongView::Event::Updated: {
                    // it may be not correct to take only visible rect
                    CGRect rect = [self.musicListView visibleRect];
                    NSIndexSet *columns = [self.musicListView columnIndexesInRect:rect];
                    
                    [self.musicListView reloadDataForRowIndexes:rows columnIndexes:columns];
                    break;
                }
                case SongView::Event::Deleted:
                    [self.musicListView beginUpdates];
                    [self.musicListView removeRowsAtIndexes:rows withAnimation:NSTableViewAnimationEffectNone];
                    [self.musicListView endUpdates];
                    break;
            }*/
            
            [self showCount:_visibleSongs.size()];
        });
        [self showCount:_visibleSongs.size()];
    }
    
        // TODO: keep selection when reload, but clean everything else:
    // keep entryId's, reload data, then keep only the entries with the right entryids selected
    // note that this could not work with the old codebase safely
    // when this is done, remove the deselectOnNextReload hack
    
    NSIndexSet *selectedRows = [musicListView selectedRowIndexes];
    __block map<int,string> entryIds;
    
    if (deselectOnNextReload) {
        [musicListView deselectAll:nil];
        deselectOnNextReload = NO;
    } else {
        [selectedRows enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
            // the playlist might have changed meanwhile
            if (idx < _visibleSongs.size()) {
                entryIds.insert(std::make_pair(idx,_visibleSongs.at(idx).entryId()));
            }
        }];
    }
    [musicListView reloadData];
    
    NSMutableIndexSet *set = [NSMutableIndexSet indexSet];
    auto num = _visibleSongs.size();
    for (const auto &p : entryIds) {
        if (p.first < num && _visibleSongs.at(p.first).entryId() == p.second) {
            [set addIndex:p.first];
        }
    }
    [musicListView selectRowIndexes:set byExtendingSelection:NO];
    
    //[self.albumViewController reload];
}

- (void)setSortDescriptorsForVisibleEntries
{
    [self.musicListView scrollRowToVisible:0];
    id codedSortDescriptors = [[NSUserDefaults standardUserDefaults] objectForKey:@"sortDescriptors"];
    NSArray *sortDescriptors = [[NSValueTransformer valueTransformerForName:NSKeyedUnarchiveFromDataTransformerName] transformedValue:codedSortDescriptors];

    auto playlist = IApp::instance()->selectedPlaylist();
    if (playlist) {
        playlist->setSortDescriptor(sortDescriptorFromArray(sortDescriptors));
    }
    [self reloadTable];
}

- (void)sortChanged:(NSNotification *)note
{
    [self setSortDescriptorsForVisibleEntries];
    deselectOnNextReload = YES;
#warning TODO: allow sort changes
    //[self reloadTable];
}

- (void)reloadTableForced
{
    // the cache must be reset since this is a bigger change, not just one row data
    visibleEntries->resetCache();
    [self reloadTable];
}

- (void)storeLoaded
{
    [self reloadTableForced];
}
         
- (void)selectSearchKey:(const string &)key
{
    //NSSearchFieldCell *cell = self.searchField.cell;
    // the cell only has a template!
    NSMenu *menu = cachedSearchMenu;
    
    int index = 0;
    if (key == "artistNorm") {
        index = 1;
    } else if (key == "albumNorm") {
        index = 2;
    } else if (key == "titleNorm") {
        index = 3;
    } else if (key == "genreNorm") {
        index = 4;
    } else if (key == "comment") {
        index = 5;
    }
    
    int i = 0;
    for (NSMenuItem *menuItem in menu.itemArray) {
        [menuItem setState:(i == index)];
        ++i;
    }
}

- (IBAction)searchChanged:(NSMenuItem *)clickedItem {
    
    NSMenu *menu = [clickedItem menu];
    cachedSearchMenu = menu;
    NSArray *items = [menu itemArray];;
    for (NSMenuItem *menuItem in items) {
        [menuItem setState:(menuItem == clickedItem)];
    }
    switch([items indexOfObject:clickedItem]) {
        case 1:
            searchCategory = @"artistNorm";
            break;
        case 2:
            searchCategory = @"albumNorm";
            break;
        case 3:
            searchCategory = @"titleNorm";
            break;
        case 4:
            searchCategory = @"genreNorm";
            break;
        case 5:
            searchCategory = @"comment";
            break;
        default:
            searchCategory = nil;
    }
    [self search:searchField];
}

- (void)togglePlay:(id)sender
{
    [self play:sender];
}

- (void)statusTextClicked:(id)sender
{
    [self.mainWindow makeKeyAndOrderFront:nil];
    [self.mainWindow orderFrontRegardless];
}

- (void)playFirst
{
    [self playFirstForced:NO];
}

- (void)playFirstForced:(BOOL)forced
{
    if (forced || IApp::instance()->selectedPlaylist() == nullptr) {
        
        SongEntry song;
        NSInteger count = [self numberOfRowsInTableView:musicListView];
        if (count == 0) {
            playFirstPending = YES;
            return;
        }
        if (![self isShuffle]) {
            song = [self objectAtIndex:0];
        } else {
            song = [self objectAtIndex:arc4random_uniform((unsigned int)count)];
        }
        [self userInitiatedPlaySong:song];
    }
}

- (IBAction)play:(id)sender
{
    auto player = IApp::instance()->player();
    if (player->songEntryConnector().value()) {
        player->play();
    } else {
        NSInteger selectedRow = [self.musicListView selectedRow];
        if (selectedRow != -1) {
            [self userInitiatedPlaySong:[self objectAtIndex:selectedRow]];
        } else {
            [self playFirstForced:YES];
        }
    }
}

- (IBAction)prev:(id)sender
{
    IApp::instance()->player()->prev();
}

- (IBAction)settingsClicked:(id)sender {
}

- (IBAction)refresh:(id)sender {
}


- (IBAction)jumpToSong:(id)sender
{
    [self jumpToSong];
}

- (void)jumpToSong
{
    auto player = IApp::instance()->player();
    auto playing = player->songEntryConnector().value();
    auto playlistCurrentlyPlaying = player->playlistCurrentlyPlaying();
    
    if (!playlistCurrentlyPlaying) {
        return;
    }
    //BOOL browserModeRefresh = NO;
    
    BOOL animate = NO;
    if (IApp::instance()->selectedPlaylist() == playlistCurrentlyPlaying) {
        animate = YES;
    } else {
        IApp::instance()->userSelectedPlaylist(playlistCurrentlyPlaying);
    }
    playlistCurrentlyPlaying->selectGrouping(player->groupingCurrentlyPlaying(), false);
    dispatch_async(dispatch_get_main_queue(),^{
        [self jumpToSongForSong:playing animate:animate userInitiated:YES];
    });
 
}

- (void)jumpToSongForSong:(SongEntry)song animate:(BOOL)animate userInitiated:(BOOL)userInitiated
{
    if (!userInitiated) {
        // if not moved, keep following
        userInitiated = ![self userMoved];
    }

    // not at top
    BOOL songStillValid = [self isSongStillValid:song];
    
    // && CGRectGetMinY([musicListViewScroll documentVisibleRect]) != 0 caused a mysterious bug, so we certainly don't want it
    if (songStillValid || (!userInitiated && CGRectGetMinY([musicListViewScroll documentVisibleRect]) != 0)) {
        
        NSUInteger index = [self indexOfObject:song];
        if (index == NSNotFound) {
            return;
        }
        //NSAssert(song == [visibleEntries objectAtIndex:index], @"indexofobject must find the right song!");
        
        lastScroll = 0;
        
        // an animated [musicListView scrollRowToVisible:index];
        NSRect rowRect = [musicListView rectOfRow:index];
        NSRect viewRect = [[musicListView superview] frame];
        NSPoint scrollOrigin = rowRect.origin;
        
        const CGFloat headerHeight = musicListView.headerView.frame.size.height;
        const CGFloat unUsefulHeight = (translucencyOffset + headerHeight);
        // very important to use integer numbers, otherwise glitches occur
        // (2*rowheight is just to make the centering appear a bit upper)
        scrollOrigin.y = (int)(scrollOrigin.y + (rowRect.size.height - (viewRect.size.height - unUsefulHeight - 2*rowRect.size.height)) / 2);
        if (scrollOrigin.y < 0) {
            scrollOrigin.y = 0;
        }
        id executeOnObject = [musicListView superview];
        if (animate) {
            executeOnObject = [executeOnObject animator];
        }
        scrollOrigin.y -= unUsefulHeight;
        [executeOnObject setBoundsOrigin:scrollOrigin];
            
        if (userInitiated) {
            userInitiatedScrollOrigin = scrollOrigin.y;
        }
    }
}

- (CGFloat)currentScrollOrigin
{
    return [[musicListView superview] bounds].origin.y;
}

- (BOOL)isShuffle
{
    return [shuffleButton intValue];
}

- (BOOL)isContinous
{
    return [repeatButton intValue] > 0;
}

- (shared_ptr<ISong>)currentlyPlaying
{
    return PlaybackController::instance()->songPlayed();
}

- (IBAction)shufflePressed:(id)sender
{
    // can be a button or a menuitem
    NSInteger value = 0;
    if ([sender respondsToSelector:@selector(intValue)]) {
        value = [sender intValue];
    } else {
        value = ![(NSMenuItem *)sender state];
    }
    IApp::instance()->player()->shuffle() = value;
}

- (IBAction)outputsPressed:(id)sender
{
    [[OutputsController sharedController] show:sender];
}

- (IBAction)repeatPressed:(id)sender
{
    NSInteger value = 0;
    if ([sender respondsToSelector:@selector(intValue)]) {
        value = [sender intValue];
    } else {
        value = ![(NSMenuItem *)sender state];
    }
    IApp::instance()->player()->repeat() = static_cast<IPlayer::Repeat>(value);
}


- (IBAction)next:(id)sender {
    //NSLog(@"DEBUG songplayed: %@", [PlaybackController sharedController].songPlayed);
    
    IApp::instance()->player()->next();
}

- (BOOL)isCachedSongStillValid
{
    if (!cachedSong) {
        return false;
    }
    return [self isSongStillValid:cachedSong];
}

- (BOOL)isSongStillValid:(SongEntry)song
{
    if (!song) {
        return false;
    }
    return visibleEntries->contains(song);
}

- (IBAction)sendFeedback:(id)sender {
    
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.gearmusicplayer.com/help.html#contactfromapp"]];
}

- (BOOL)windowShouldClose:(id)sender
{
    [mainWindow orderOut:sender];
    [mainWindow setExcludedFromWindowsMenu:NO];
    return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
    //NSLog(@"resize: %@ %@ >> %@", self.mainWindow.contentView, NSStringFromRect([self.mainWindow frame]), NSStringFromRect([self.mainWindow.contentView frame]));
    
    [self resizeTopViews];

 	NSView *view = self.mainWindow.contentView;
	view.frame = (NSRect) {0, 0, mainWindow.frame.size};
    // autoresize does not work!
    [self centerBigMessage];
    
    
    if (IApp::instance()->player()) {
        bool playing = IApp::instance()->player()->playingConnector().value();
        //NSLog(@"playing: %d", playing);
    }
}

- (void)setMusicListContentInsets
{
    if ([self.musicListViewScroll respondsToSelector:@selector(setContentInsets:)]) {
        
        void(^set)(NSScrollView *) = ^(NSScrollView *view){
            view.automaticallyAdjustsContentInsets = NO;
            view.contentInsets = NSEdgeInsetsMake(translucencyOffset, 0, 0, 0);
        };
        set(self.musicListViewScroll);
        set(self.imageBrowserView.enclosingScrollView);
    }
}

- (void)updatePlayCenter
{
    NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
    const CGFloat offset = (prefix == nil && ![self.playButton intValue]) ? 2 :0;
    CGRect frame = self.playButton.frame;
    frame.origin.x = (int)(CGRectGetMaxX(self.prevButton.frame) + (CGRectGetMinX(self.nextButton.frame) - CGRectGetMaxX(self.prevButton.frame) -frame.size.width )/2 + offset);
    self.playButton.frame = frame;
    
    if (prefix != nil && ![self.playButton intValue]) {
        [[playButton cell] setXOffset:1];
    } else {
        [[playButton cell] setXOffset:0];
    }
}

- (void)setupTopLeft
{
    using namespace sfl;
    
    vector<NSView *> chain;
    chain.push_back(self.prevButton);
    chain.push_back(self.playButton);
    chain.push_back(self.nextButton);
    
    auto sideConstraints = [](const std::tuple<NSView *,NSView *,NSView *> &t)->std::pair<NSLayoutConstraint *,NSLayoutConstraint *>{
        // tuple members are view, previous, following
        NSView *view = std::get<0>(t);
        view.translatesAutoresizingMaskIntoConstraints = NO;
        NSView *container = view.superview;
        
        //[container addConstraint:[NSLayoutConstraint constraintWithItem:view attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:container attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
        
        NSView *previous = std::get<1>(t);
        if (previous == nil) {
            previous = container;
        }
        NSView *following = std::get<2>(t);
        
        NSLayoutAttribute prevAttribute = (previous == container) ? NSLayoutAttributeLeft : NSLayoutAttributeRight;
        NSLayoutAttribute nextAttribute = NSLayoutAttributeLeft;
        
        NSLayoutConstraint *zeroLeft = [NSLayoutConstraint constraintWithItem:view attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:previous attribute:prevAttribute multiplier:1 constant:0];
        zeroLeft.priority = NSLayoutPriorityDefaultHigh;
        [container addConstraint:zeroLeft];
        
        NSLayoutConstraint *left = [NSLayoutConstraint constraintWithItem:view attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:previous attribute:prevAttribute multiplier:1 constant:0];
        [container addConstraint:left];
        NSLayoutConstraint *right = nil;
        
        if (following) {
            NSLayoutConstraint *zeroRight = [NSLayoutConstraint constraintWithItem:view attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:following attribute:nextAttribute multiplier:1 constant:0];
            zeroRight.priority = NSLayoutPriorityDefaultHigh;
            [container addConstraint:zeroRight];
            
            right = [NSLayoutConstraint constraintWithItem:following attribute:nextAttribute relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:view attribute:NSLayoutAttributeRight multiplier:1 constant:0];
            [container addConstraint:right];
        }
        
        return std::make_pair(left, right);
    };
    NSView *nilView = nil;
    marginConstraints = sfl::Map::fromRange(sfl::map([&](auto t){return std::make_pair(std::get<0>(t),sideConstraints(t));}, zip3(chain,cons(nilView,chain),tail(snoc(chain,nilView)))));
}

- (void)resizeTopViews
{
    //NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
    CGRect frame;
   
    /*frame = self.prevButton.frame;
    frame.origin.x = 88;
    
    self.prevButton.frame = frame;
    frame = self.playButton.frame;
    frame.origin.x = self.prevButton.frame.origin.x + 36;
    self.playButton.frame = frame;
    frame = self.nextButton.frame;
    frame.origin.x = self.playButton.frame.origin.x + 40;
    self.nextButton.frame = frame;
  */
    static BOOL topLeftInited = NO;
    if (!topLeftInited && self.nextButton != nil) {
        [self setupTopLeft];
        topLeftInited = YES;
    }
    
    [self updatePlayCenter];
    
    //frame = self.volumeHolder.frame;
    //frame.origin.x = self.nextButton.frame.origin.x + 47;
    //self.volumeHolder.frame = frame;
    
    
    const CGFloat sliderSpacer = 3.0f;
    const CGFloat textSpacer = 5.0f + 23.0f;
    
    //NSDictionary *attr = [NSDictionary dictionaryWithObject:songTitle.font forKey:NSFontAttributeName];
    //const CGFloat textMin = MAX(MAX(200.0f, [songTitle.stringValue sizeWithAttributes:attr].width), [songAlbumArtist.stringValue sizeWithAttributes:attr].width);
    const CGFloat textMin = 200.0f;
    const CGFloat sliderMin = 104.0f;
    const CGFloat sliderMax = 200.0f;
    
    // let's scale nicely the album, artist, title, song seeker views, just like as with autolayout!
    //CGFloat begin = nextButton.frame.origin.x + nextButton.frame.size.width + textSpacer + 20;
    // playback controls should be autoresized, therefore unrelated to this until we autoresize this as well
    CGFloat begin = 248;
    CGFloat end;
    
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:kUserDefaultsShowVolume]) {
        
        begin = self.volumeHolder.frame.origin.x + self.volumeHolder.frame.size.width + textSpacer;
    }
    
    end = [topContainer convertRect:self.searchField.frame fromView:self.searchField.superview].origin.x - 58.0f - sliderSpacer;
    
    CGFloat total = end-begin - (textSpacer + sliderSpacer) - songElapsed.frame.size.width;
    CGFloat sliderWidth;
    sliderWidth = total - textMin;
    if (sliderWidth > sliderMax) {
        sliderWidth = sliderMax;
    } else if (sliderWidth < sliderMin) {
        sliderWidth = sliderMin;
    }
    CGFloat textWidth = total - sliderWidth;

    // an offset for horizontally center positioning (while aligned to left!)
    //CGFloat textReal = MAX(songTitle.attributedStringValue.size.width, songAlbumArtist.attributedStringValue.size.width);
    //textReal = MIN(textReal, textWidth);
    
    //int textOffset = (textWidth - textReal)/2;
    
    //CGFloat kOffset = 10;
    
    // autolayout instead:
    
    // font sizes are different, so we need a different offset...
    //CGFloat timeOriginY = playbackPositionSlider.frame.origin.y + (yosemiteOrBetter() ? -1 : 1);
    // 20.0f accounts for this view being on the outer 'rim', see topMostViews
    //songTitle.frame       = CGRectMake(begin - 20.0f + kOffset + textOffset, songTitle.frame.origin.y,
    //                                  textWidth-textOffset, songTitle.frame.size.height);
    //songAlbumArtist.frame = CGRectMake(begin - 20. + kOffset + textOffset, songAlbumArtist.frame.origin.y,
    //                                   textWidth-textOffset, songAlbumArtist.frame.size.height);
    //songElapsed.frame   = CGRectMake(begin + textWidth + textSpacer + 8.0f, timeOriginY,
    //                                   songElapsed.frame.size.width, songElapsed.frame.size.height);
    playbackPositionSlider.frame = CGRectMake(end - sliderWidth + 5.0f, playbackPositionSlider.frame.origin.y,
                                              sliderWidth, playbackPositionSlider.frame.size.height);
    
    //songRemaining.frame = CGRectMake(end + 5.0f, timeOriginY, songRemaining.frame.size.width, songRemaining.frame.size.height);
}

- (void)setupSharing
{
    BOOL showShare;
    if (!mountainLionOrBetter()) {
        showShare = NO;
    }
    
    showShare = [[NSUserDefaults standardUserDefaults] boolForKey:kUserDefaultsShowShareButton];
    
    NSView *shareButton = self.shareButton;
    NSDictionary *views = NSDictionaryOfVariableBindings(topContainer,shareButton);
    
    self.shareButton.translatesAutoresizingMaskIntoConstraints = NO;
    static BOOL alreadySet = NO;
    if (!alreadySet) {
        alreadySet = YES;
        [topView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[shareButton(==19)]" options:0 metrics:nil views:views]];
        [topView addConstraint:[NSLayoutConstraint constraintWithItem:shareButton attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:topContainer attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    }
    
    static NSArray *sharePresent = [NSLayoutConstraint constraintsWithVisualFormat:@"|[topContainer]-(==0)-[shareButton(==34)]-|" options:0 metrics:nil views:views];
    static NSArray *shareHidden = [NSLayoutConstraint constraintsWithVisualFormat:@"|[topContainer]|" options:0 metrics:nil views:views];
    [topView removeConstraints:sharePresent];
    [topView removeConstraints:shareHidden];
    if (showShare) {
        //[topView removeConstraints:shareHidden];
        [topView addConstraints:sharePresent];
        [shareButton setHidden:NO];
    } else {
        //[topView removeConstraints:sharePresent];
        [topView addConstraints:shareHidden];
        [shareButton setHidden:YES];
    }
    [topView setNeedsLayout:YES];
    
    /*
    CGRect frame = topContainer.frame;
    CGRect searchFrame = self.searchField.frame;
    CGFloat placeForShareButton = 52.0f;
    if (showShare) {
        frame.size.width = self.mainWindow.frame.size.width - frame.origin.x - placeForShareButton - 11.0f;
    } else {
        frame.size.width = self.mainWindow.frame.size.width - frame.origin.x - 16.0f;
    }
    searchFrame.origin.x = frame.size.width - 104 -56;
    topContainer.frame = frame;
    self.searchField.frame = searchFrame;
    self.searchFieldCustom.frame = searchFrame;
    
    frame.origin.x = topContainer.frame.origin.x + topContainer.frame.size.width + placeForShareButton/2 - self.shareButton.frame.size.width / 2 - 6.0f;
    
    NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];

    //frame.origin.y = topContainer.frame.origin.y + topContainer.frame.size.height/2 - self.shareButton.frame.size.height/2 + 3.0f - ((prefix == nil) ? 0 : 1) + 8;
    //frame.origin.y = self.shareButton.superview.frame.size.height - (int)((kTopHeightFullScreen + self.shareButton.frame.size.height)/2) - ((prefix == nil) ? 1 : 2);
    frame.origin.y += 12;;
    frame.size = self.shareButton.frame.size;
    
    [self.shareButton setFrame:frame];
    [self.shareButton setAutoresizingMask:NSViewMinYMargin | NSViewMinXMargin];*/
    [self setShareImage];
    [self.shareButton setTarget:self];
    [self.shareButton setAction:@selector(sharePressed:)];
    [self.shareButton sendActionOn:NSLeftMouseDownMask];
}

- (void)setShareImage
{
    [self.shareButton.cell setSystemSupplied:YES];
    [self.shareButton setImage:[NSImage imageNamed:NSImageNameShareTemplate]];
    //[self.shareButton setImage:[NSImage imageNamed:NSImageNameShareTemplate tintWithColor:[[CocoaThemeManager sharedManager] buttonContentColor]]];
}

- (void)sharePressed:(NSButton *)sender
{
    shared_ptr<ISong> song = PlaybackController::instance()->songPlayed();
    if (!song) {
        return;
    }
    
    if (shareManager == nil) {
        shareManager = [[ShareManager alloc] init];
    }
    [shareManager setSong:song];
    [shareManager shareFromView:sender];    
}

static NSString *kSongKey = @"song";

- (void)sendToNotificationCenter:(shared_ptr<ISong>)song
{
    NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];
    NSUserDefaultsController *userDefaults = [NSUserDefaultsController sharedUserDefaultsController];
    if ([[[userDefaults values] valueForKey:kNotificationCenterEnabledOnlyLast] boolValue]) {
        [center removeAllDeliveredNotifications];
    }
    
    if (!song || equals(lastNotificationSong, song)) {
        return;
    }
    lastNotificationSong = song;
    
    NSUserNotification *notification = [[NSUserNotification alloc] init];
    notification.subtitle = convertString(song->artist());
    notification.informativeText = convertString(song->album());
    notification.title = convertString(song->title());
    notification.hasActionButton = YES;
    notification.actionButtonTitle = @"Skip";
    notification.userInfo = [NSDictionary dictionaryWithObject:convertString(song->uniqueId()) forKey:kSongKey];
    if ([notification respondsToSelector:@selector(setContentImage:)]) {
        static shared_ptr<PromisedImage> image;
        image = IApp::instance()->albumArtStash()->art(song, 128);
        if (image != nullptr) {
            static SignalConnection notificationAlbumConnection;
            notificationAlbumConnection = image->connector().connect([self, notification](const shared_ptr<IPaintable> &paintable){
                if (paintable) {
                    NSImage *image = Painter::convertImage(*paintable);
                    [notification setContentImage:image];
                }
            });
        }
    }
    
    [center setDelegate:self];
    // a little delay is needed to fetch album arts
    double delayInSeconds = 1.0f;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [center deliverNotification:notification];
    });
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
    return [[[notification userInfo] objectForKey:@"forceShow"] boolValue];
}

- (void)doSetAlbumArtImage:(NSImage *)image
{
    [image setSize:NSMakeSize(0, 0)];
    [self.albumArtImageWell setImage:image];
    
    BOOL showImageInDock = [[NSUserDefaults standardUserDefaults] boolForKey:@"ShowAlbumArtInDock"];
    if (showImageInDock) {
        [[NSApplication sharedApplication] setApplicationIconImage:image];
    }
}

- (void)displayAlbumArtForSong:(shared_ptr<ISong>)desired
{
    CGRect imageFrame = albumArtImageWell.frame;
    if ([[albumArtImageWell window] respondsToSelector:@selector(convertRectToBacking:)]) {
        imageFrame = [[albumArtImageWell window] convertRectToBacking:imageFrame];
    }
    CGFloat dimension = MAX(imageFrame.size.width, imageFrame.size.height);
    
    displayAlbum = IApp::instance()->albumArtStash()->art(desired, dimension);
    if (displayAlbum != nullptr) {
        displayAlbumConnection = displayAlbum->connector().connect([self](const shared_ptr<IPaintable> &paintable){
            if (paintable) {
                NSImage *image = Painter::convertImage(*paintable);
                [self doSetAlbumArtImage:image];
            } else {
                [self doSetAlbumArtImage:nil];
            }
        });
    }
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    if (notification.activationType == NSUserNotificationActivationTypeActionButtonClicked && [[notification.userInfo objectForKey:kSongKey] isEqualTo:convertString(PlaybackController::instance()->songPlayed()->uniqueId())]) {
        
        [self next:nil];
    }
    [[NSUserNotificationCenter defaultUserNotificationCenter] removeDeliveredNotification:notification];
}

- (void)reloadVisibleSongs
{
    CGRect rect = [self.musicListView visibleRect];
    NSIndexSet *columns = [self.musicListView columnIndexesInRect:rect];
    NSRange rowsRange = [self.musicListView rowsInRect:rect];
    NSIndexSet *rows = [NSIndexSet indexSetWithIndexesInRange:rowsRange];
    
    [self.musicListView reloadDataForRowIndexes:rows columnIndexes:columns];
}

- (NSTableRowView *)tableView:(NSTableView *)tableView
                rowViewForRow:(NSInteger)row
{
    // 'position' must be rendered differently, since it is out of the regular bounds
    NSString * const identifier = @"Row";
    SongsRowView *view = [tableView makeViewWithIdentifier:identifier owner:self];
    if (view == nil) {
        view = [[SongsRowView alloc] init];
        view.tableView = tableView;
        view.identifier = identifier;
    }
    
    NSScrollView* scrollView = [self.musicListView enclosingScrollView];
    CGRect visibleRect = scrollView.contentView.visibleRect;
    NSRange range = [self.musicListView rowsInRect:visibleRect];
    _visibleSongs.setVisibleRange(range.location, range.length);
    
    view.row = row;
    SongEntry entry = _visibleSongs.at(row);
    view.entry = entry;
    
    return view;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    // view-based tableviews
    
    NSTableCellView *cellView = nil;
    NSView *field = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
    if (field == nil) {
        if ([tableColumn.identifier isEqualToString:@"source"]) {
            field = [[SourceView alloc] init];
        } else {
            if ([tableColumn.identifier isEqualToString:@"rating"]) {
                NSControl *control;
                control = [[NSControl alloc] init];
                control.cell = [self tableView:tableView dataCellForTableColumn:tableColumn row:row];
                field = control;
            } else {
                cellView = [[NSTableCellView alloc] init];
                NSControl *control = [[AlignedTextField alloc] init];
                control.cell = [self tableView:tableView dataCellForTableColumn:tableColumn row:row];
                [[control cell] setLineBreakMode:NSLineBreakByTruncatingTail];
                [[control cell] setFocusRingType:NSFocusRingTypeNone];
                //cellView.textField = (NSTextField *)control;
                control.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
                // without this we can not save data:
                control.identifier = tableColumn.identifier;
                [cellView addSubview:control];
                control.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
                //control.wantsLayer = YES;
                
                [(NSTextField *)control setDelegate:self];
                field = cellView;
            }
            
        }
        field.identifier = tableColumn.identifier;
        cellView.identifier = tableColumn.identifier;
        field.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
    }
    
    if ([tableColumn.identifier isEqualToString:@"source"]) {
        
        SongEntry entry = [self objectAtIndex:row];
        auto song = entry.song();
        
        //NSLog(@"row: %ld song: %@", (long)row, convertString(song->title()));
        [(SourceView *)field setSong:song];
    } else {
        NSControl *control = (NSControl *)field;
        if ([control isKindOfClass:[NSTableCellView class]]) {
            cellView = (NSTableCellView *)control;
            control = [[cellView subviews] objectAtIndex:0];
        }
        [self tableView:tableView willDisplayCell:control.cell forTableColumn:tableColumn row:row];
        [control.cell setEditable:[self tableView:tableView shouldEditTableColumn:tableColumn row:row]];
        NSString *str = [self tableView:tableView objectValueForTableColumn:tableColumn row:row];
        if (str != nil) {
            [control setStringValue:str];
        }
        
        if ([control.cell respondsToSelector:@selector(setSuperBackgroundColor:)]) {
            [control.cell setSuperBackgroundColor:[[tableView selectedRowIndexes] containsIndex:row] ? [NSColor alternateSelectedControlColor] : [[[CocoaThemeManager sharedManager] songsAlternatingColors] objectAtIndex:row %2]];
        }
    }
    
    if (cellView) {
        return cellView;
    }
    return field;
}

+ (void)refreshTableTextBackground:(NSView *)field row:(NSInteger)row table:(NSTableView *)table
{
    if ([field.identifier isEqualToString:@"source"]) {
        return;
    }
    
    NSView *cellView;
    NSControl *control = (NSControl *)field;
    if ([control isKindOfClass:[NSTableCellView class]]) {
        cellView = (NSTableCellView *)control;
        control = [[cellView subviews] objectAtIndex:0];
    }
    if ([control.cell respondsToSelector:@selector(setSuperBackgroundColor:)]) {
        [control.cell setSuperBackgroundColor:[[table selectedRowIndexes] containsIndex:row] ? [NSColor alternateSelectedControlColor] : [[[CocoaThemeManager sharedManager] songsAlternatingColors] objectAtIndex:row %2]];
    }
}

- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor
{
    return YES;
}

- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
    if (![control isKindOfClass:[NSTextField class]]) {
        return YES;
    }
    NSTextField *textField = (NSTextField *)control;
    NSString *value = [textField stringValue];
    if (!value) {
        return YES;
    }
    
    SongListTextFieldCell *cell = [textField cell];
    if (![cell isKindOfClass:[SongListTextFieldCell class]]) {
        return YES;
    }
    
    [self tableView:self.musicListView setObjectValue:value forTableColumn:[self.musicListView tableColumnWithIdentifier:textField.identifier] row:cell.row];
    
    return YES;
}

- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)row
{
    NSCell *ret = nil;
    NSString *identifier = [aTableColumn identifier];
    if ([identifier isEqualToString:@"rating"]) {
        RatingCell *cell = [[RatingCell alloc] init];
        cell.tableView = self.musicListView;
        ret = cell;
    } else if ([identifier isEqualToString:@"currentInstant"])  {
        ret = [[InstantCell alloc] init];
    } else if ([identifier isEqualToString:@"position"]) {
        TextImageFieldCell *cell = [[TextImageFieldCell alloc] init];
        cell.tableView = self.musicListView;
        ret = cell;
    } else {
        SongListTextFieldCell *cell = [[SongListTextFieldCell alloc] init];
        ret = cell;
        cell.tableView = self.musicListView;
        cell.usesSingleLineMode = YES;
    }
    return ret;
}


- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    if (!selectionReload) {
        selectionReload = [NSIndexSet indexSet];
    }
    NSMutableIndexSet *sum = [selectionReload mutableCopy];
    [sum addIndexes:[self.musicListView selectedRowIndexes]];
    
    NSInteger col = [self.musicListView columnWithIdentifier:@"rating"];
    if (col != NSNotFound) {
        // otherwise stars won't update correctly
        [self.musicListView reloadDataForRowIndexes:sum columnIndexes:[NSIndexSet indexSetWithIndex:col]];
    }
    
    // these are very slow:
    //[self.musicListView reloadDataForRowIndexes:sum columnIndexes:[self.musicListView columnIndexesInRect:[self.musicListView visibleRect]]];
    //[self.musicListView reloadDataForRowIndexes:[self.musicListView selectedRowIndexes] columnIndexes:[self.musicListView columnIndexesInRect:[self.musicListView visibleRect]]];
}

- (void)tableView:(SongsTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	//NSLog(@"WILLDISPLAY");
    SongEntry entry = [self objectAtIndex:rowIndex];
    auto song = entry.song();
    
    if ([aCell respondsToSelector:@selector(setRow:)]) {
        [(SongListTextFieldCell *)aCell setRow:rowIndex];
    }
    if ([aCell respondsToSelector:@selector(setColumn:)]) {
        [(SongListTextFieldCell *)aCell setColumn:[aTableView.tableColumns indexOfObject:aTableColumn]];
    }
    
    if ([[aTableColumn identifier] isEqualToString:@"rating"]) {
        BOOL highlighted = [[aTableView selectedRowIndexes] containsIndex:rowIndex];
        [aCell setHighlighted:highlighted];
        [(RatingCell *)aCell setSong:song];
    } else if ([[aTableColumn identifier] isEqualToString:@"currentInstant"])  {
        [(InstantCell *)aCell setSong:song];
	} else if ([[aTableColumn identifier] isEqualToString: @"source"]) {
		// we manage the view directly
    } else {
        static dispatch_once_t onceToken;
        static NSFont *plainFont;
        static NSFont *boldFont;
        dispatch_once(&onceToken, ^{
            
            plainFont = Gui::Writer::convertFont(IApp::instance()->themeManager()->current()->listTitleText().font());
            plainFont = [NSFont fontWithName:[plainFont fontName] size:[plainFont pointSize] + kEnlargeTableText];
            boldFont = [[NSFontManager sharedFontManager] convertFont:plainFont toHaveTrait:NSBoldFontMask];
        });
        
        if ([aCell respondsToSelector:@selector(setVerticalOffset:)]) {
            [aCell setVerticalOffset:kEnlargedTableTextVerticalOffset];
        }
        
        string key = convertString([aTableColumn identifier]);
        
        if ([aCell respondsToSelector:@selector(setMayHighlightOnHover:)]) {
            [aCell setMayHighlightOnHover:(key == "artist" || key == "album" || key == "albumArtist")];
        }
        
        BOOL currentlyPlaying = IApp::instance()->player()->songEntryConnector().value() == entry;
        [aCell setFont:currentlyPlaying ? boldFont : plainFont];
        //[aCell setTextColor:color];
    }
}

- (void)tableView:(NSTableView *)tableView didDragTableColumn:(NSTableColumn *)tableColumn
{
    [self setupToggleColumnMenu];
    // SongsRowView with the now playing little icon should be redrawn
    [tableView setNeedsDisplayRecursive];
}

#pragma mark --- Datasource

- (void)setContent:(shared_ptr<ISongArray>)aContent
{
    visibleEntries = aContent;
    // maybe we already have something ?
    //[self reloadTable];
    
    // capture while we still can... (it won't work well otherwise)
    void (^block)() = [^{
        [self reloadTable];
        if (playFirstPending) {
            playFirstPending = NO;
            
            [self playFirst];
        }
    } copy];
    
#warning TODO: update on search term or sort order change
    
    visibleUpdateConnection = visibleEntries->updatedEvent().connect([=]{

        dispatch_async(dispatch_get_main_queue(), block);
    });
}

- (shared_ptr<ISongArray>)content
{
    return visibleEntries;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if (visibleEntries) {
        auto ret = _visibleSongs.size();
        //std::cout << "!! row count: " << ret << std::endl;
        return ret;
    } else {
        return 0;
    }
}

- (SongEntry)objectAtIndex:(long)index
{
    if (index < 0) {
        return SongEntry();
    }
    return _visibleSongs.at(index);
}

- (NSUInteger)indexOfObject:(SongEntry)object
{
#warning TODO: index of song for scrolling
    return NSNotFound;
    //return _visibleSongs->indexOf(object);
}

static NSParagraphStyle *truncatedParagraph()
{
    NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
    paragraphStyle.lineBreakMode = NSLineBreakByTruncatingTail;
    return paragraphStyle;
}

- (id)tableView:(SongsTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
#ifdef DEBUG
    //NSLog(@"DISPLAYING: %ld", (long)rowIndex);
#endif
    
    NSString *identifier = [aTableColumn identifier];

    SongEntry entry = _visibleSongs.at(rowIndex);
    shared_ptr<ISong> object = entry.song();
    if (!object) {
        return nil;
    }
    if ([identifier isEqualToString:@"position"]) {
        // playback icon or not
        return (entry == IApp::instance()->player()->songEntryConnector().value()) ? @"volume" : nil;
    }
    static dispatch_once_t onceToken;
    static MillisecToTimeTransformer *millisecTransformer;
    static CalendarTimeTransformer *calendarTimeTransformer;
    static NumberIfNotZeroTransformer *numberIfNotZeroTransformer;
    dispatch_once(&onceToken, ^{
        millisecTransformer = [[MillisecToTimeTransformer alloc] init];
        calendarTimeTransformer = [[CalendarTimeTransformer alloc] init];
        numberIfNotZeroTransformer = [[NumberIfNotZeroTransformer alloc] init];
    });
    if ([identifier isEqualToString:@"durationMillis"]) {
        return [millisecTransformer transformedInt:object->durationMillis()];
    } else if ([identifier isEqualToString:@"creationDate"]) {
        return [calendarTimeTransformer transformedNanoSecs:object->creationDate()];
    } else if ([identifier isEqualToString:@"lastPlayed"]) {
        return [calendarTimeTransformer transformedNanoSecs:object->lastPlayed()];
    } else if ([identifier isEqualToString:@"year"] || [identifier isEqualToString:@"playCount"] || [identifier isEqualToString:@"track"] || [identifier isEqualToString:@"disc"]) {
        uint64_t value = object->uIntForKey(convertString(identifier));
        
        if (value > 0) {
            return [[NSNumber numberWithUnsignedLongLong:value] description];
        } else {
            return @"";
        }
    } else if ([identifier isEqualToString:@"currentInstant"]) {
        return @NO;
        //return [NSNumber numberWithBool:equals([[InstantManager sharedManager] currentInstant], object)];
    } else {
        
        string key = convertString(identifier);
        string strValue = object->stringForKey(key);
        NSString *ret = convertString(strValue);
        //NSLog(@"got value: %@", ret);
        
        return ret;
    }
}

- (BOOL)tableView:(NSTableView *)tableView shouldShowCellExpansionForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return YES;
}

static int lastClick = 0;
static SongEntry lastClickEntry;
static string lastClickKey;

- (void)singleClick:(id)sender
{
    return;
}

- (void)addSearchItemToContextMenu:(NSMenu *)menu
{
    lastClick++;
    //NSEvent *event = [[NSApplication sharedApplication] currentEvent];
    
    long row = [musicListView hoverRow];
    if (row >= 0) {
        NSArray *columns = musicListView.tableColumns;
        int hoverColumn = musicListView.hoverColumn;
        if (hoverColumn >= 0 && hoverColumn < [columns count]) {
            NSTableColumn *column = [columns objectAtIndex:hoverColumn];
            
            const string key = convertString([column identifier]);
            if ((key == "artist" || key == "album" || key == "albumArtist")) {
                SongEntry entry = _visibleSongs.at(row);
                
                lastClickEntry = entry;
                lastClickKey = key;
                [menu addItemWithTitle:[NSString stringWithFormat:@"Search for %@", convertString(entry.song()->stringForKey(key))] action:@selector(searchAfterSingleClick:) keyEquivalent:@""];
            }
        }
    }
}

- (void)searchAfterSingleClick:(id)sender
{
    SongEntry entry = lastClickEntry;
    if (entry) {
        IApp::instance()->showRelated(entry, lastClickKey);
    }
}

- (void)doubleClick:(id)sender
{
    lastClick++;
    
    long row = [musicListView clickedRow];
    if (row < 0) {
        return;
    }
    NSUInteger columnIndex = [musicListView clickedColumn];
    NSArray *columns = [musicListView tableColumns];
    if (columnIndex < [columns count]) {
        NSTableColumn *column = [columns objectAtIndex:columnIndex];
        if ([[column identifier] isEqualToString:@"rating"]) {
            return;
        }
    }
    
    if ([self shouldStartSong:sender row:row]) {
    
        SongEntry songToPlay = [self objectAtIndex:row];
        [self userInitiatedPlaySong:songToPlay];
    }
}

- (BOOL)tableView:(SongsTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
{
    selectionReload = [aTableView selectedRowIndexes];
    
    NSInteger column = [aTableView clickedColumn];
    if (column == -1) {
        // arrow key navigation
        return YES;
    }
    if (column == NSNotFound || [[[[aTableView tableColumns] objectAtIndex:column] identifier] isEqualTo:@"rating"]) {
        return NO;
    }
    return YES;
}

- (BOOL)shouldStartSong:(SongsTableView *)aTableView row:(NSInteger)rowIndex
{
    return YES;
}

- (void)setupToggleColumnMenu
{
    if (columnsShouldBeHidden == nil) {
        columnsShouldBeHidden = [[[NSUserDefaults standardUserDefaults] arrayForKey:@"HiddenColumns"] mutableCopy];
        if (columnsShouldBeHidden == nil) {
            columnsShouldBeHidden = [[NSMutableArray alloc] init];
        }
    } else {
        for (NSTableColumn *column in musicListView.tableColumns) {
            if ([column isHidden] && ![columnsShouldBeHidden containsObject:[column identifier]]) {
                [column setHidden:NO];
            }
        }
    }
    
    NSMutableArray *toHideArray = [[NSMutableArray alloc] init];
    for (const string &toHide : visibleEntries->forbiddenColumns()) {
        NSString *str = convertString(toHide);
        [toHideArray addObject:str];
        for (NSTableColumn *column in musicListView.tableColumns) {
            NSString *columnKey = [column identifier];
            if ([columnKey isEqualToString:str]) {
                [column setHidden:YES];
            }
        }
    }
    
    NSMenu *menu = [[NSMenu alloc] initWithTitle:@""];
    for (NSTableColumn *column in musicListView.tableColumns) {
        NSString *title = [[column headerCell] stringValue];
        if ([title isEqualToString:@"#"] || [toHideArray containsObject:[column identifier]]) {
            continue;
        }
        NSMenuItem *menuItem = [menu addItemWithTitle:title action:@selector(toggleColumnFromMenu:) keyEquivalent:@""];
        menuItem.target = self;
        menuItem.state = ![column isHidden];
    }
    musicListView.headerView.menu = menu;
}

- (void)toggleColumnFromMenu:(NSMenuItem *)menuItem
{
    for (NSTableColumn *column in musicListView.tableColumns) {
        NSString *title = [[column headerCell] stringValue];
        
        if ([title isEqualToString:menuItem.title]) {
            [column setHidden:![column isHidden]];
            if (![column isHidden]) {
                [columnsShouldBeHidden removeObject:[column identifier]];
            } else {
                [columnsShouldBeHidden addObject:[column identifier]];
            }
            [[NSUserDefaults standardUserDefaults] setObject:columnsShouldBeHidden forKey:@"HiddenColumns"];
            [self setupToggleColumnMenu];
            break;
        }
    }
}

- (void)copySelectionToClipboard
{
    NSPasteboard *pasteBoard = [NSPasteboard generalPasteboard];
    
    long row = [musicListView selectedRow];
    if (row < 0) {
        return;
    }
    
    SongEntry selectedEntry = [self objectAtIndex:row];
    shared_ptr<ISong> selected = selectedEntry.song();
    
    NSString *stringRepresentation;
    if (!selected) {
        return;
    }
    if (!selected->artist().empty()) {
        if (!selected->title().empty()) {
            stringRepresentation = [NSString stringWithFormat:@"%@ - %@", convertString(selected->artist()), convertString(selected->title())];
        } else {
            stringRepresentation = convertString(selected->artist());
        }
    } else {
        stringRepresentation = convertString(selected->title());
    }
    [pasteBoard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
    [pasteBoard setString:stringRepresentation forType:NSStringPboardType];
}

- (void)contextMenuIntent:(id)sender
{
    if ([sender isKindOfClass:[MenuItemWithIntent class]]) {
        MenuItemWithIntent *menuItem = sender;
        auto intent = menuItem.intent;
        if (intent) {
            vector<SongEntry> entries = contextMenuSongs;
            if (!entries.empty()) {
                
                bool confirmationNeeded = intent->confirmationNeeded(entries);
                if (confirmationNeeded) {
                    NSInteger result = NSRunAlertPanel(@"Alert", @"%@", @"Sure", @"No", nil, convertString(intent->confirmationText(entries)));
                    if (result != NSAlertDefaultReturn) {
                        return;
                    }
                    deselectOnNextReload = YES;
                    //[self reloadTable];
                }
                
                intent->apply(entries);
            }
        }
    }
}

- (NSMenu *)tableView:(NSTableView *)tableView menuForRows:(NSIndexSet *)rows column:(NSUInteger)column
{
    [self.mainWindow temporaryDisableEditing];
    
    NSMenu *menu = [[NSMenu alloc] init];
    __block vector<SongEntry> songs;
    songs.reserve([rows count]);
    [rows enumerateIndexesUsingBlock:^(NSUInteger rowIndex, BOOL *stop) {
        SongEntry object = _visibleSongs.at(rowIndex);
        if (object) {
            songs.push_back(object);
        }
    }];
    
    contextMenuSongs = songs;
    
    if (songs.size() > 0) {
        
        for (auto &menuIt : visibleEntries->songIntentSubMenus(songs)) {
            NSMenuItem *sub = [menu addItemWithTitle:convertString(menuIt.first) action:nil keyEquivalent:@""];
            NSMenu *subMenu = [[NSMenu alloc] initWithTitle:@""];
            sub.submenu = subMenu;
            
            for (auto &intent : menuIt.second) {
                MenuItemWithIntent *menuItem = [[MenuItemWithIntent alloc] initWithTitle:convertString(intent->menuText()) action:@selector(contextMenuIntent:) keyEquivalent:@""];
                menuItem.intent = intent;
                [subMenu addItem:menuItem];
            }
        }
        NSArray *columns = [tableView tableColumns];
        string fieldHint;
        if (column < [columns count]) {
            fieldHint = convertString([[columns objectAtIndex:column] identifier]);
        }
            
        auto intents = visibleEntries->songIntents(songs, fieldHint);
        for (auto &intent : intents) {
            MenuItemWithIntent *menuItem = [[MenuItemWithIntent alloc] initWithTitle:convertString(intent->menuText()) action:@selector(contextMenuIntent:) keyEquivalent:@""];
            menuItem.intent = intent;
            [menu addItem:menuItem];
        }
        
        if (songs.size() == 1) {
            [self addSearchItemToContextMenu:menu];
        }
    }
    return menu;
}

- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    shared_ptr<ISong> song = _visibleSongs.at(rowIndex).song();
    if (!song) {
        return NO;
    }
    BOOL ret = song->editable(convertString([aTableColumn identifier]));
    return ret;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    shared_ptr<ISong> song = _visibleSongs.at(rowIndex).song();
    if (!song) {
        return;
    }
    auto identifier = convertString([aTableColumn identifier]);
    
    
    if ([anObject isKindOfClass:[NSString class]]) {
        song->setStringForKey(identifier, convertString((NSString *)anObject));
    }
    if ([anObject isKindOfClass:[NSNumber class]]) {
        song->setUintForKey(identifier, [anObject unsignedLongLongValue]);
    }
    song->updateInDbAndRemote({identifier});
}

- (void)addToQueue:(id)sender
{
 //   if ([contextMenuSong managedObjectContext] == nil) {
        return;
   // }
    
   // NSManagedObjectContext *context = [AppDelegate sharedDelegate].remoteContextMain;
    
}

- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
        
    __block vector<SongEntry> songs;
    
    [rowIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
        
        SongEntry object = [self objectAtIndex:idx];
        if (!object) {
            return;
        }
        songs.push_back(object);
        
        NSPasteboardItem *pasteItem = [[NSPasteboardItem alloc] init];
        [pasteItem setString:convertString(object.entryId()) forType:kPasteBoardTypeSong];
        [pboard writeObjects:@[pasteItem]];

    }];
    
    App::instance()->dragManager().dragSongs(visibleEntries, songs);
    
    return YES;
}

- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
    auto playlist = IApp::instance()->selectedPlaylist();

    // assume that playlist entry is from the same playlist
    auto &dragManager = Gear::App::instance()->dragManager();
    auto draggedSongs = dragManager.draggedEntries();
    
    
    
    NSInteger numberOfRows = [self numberOfRowsInTableView:aTableView];
    if (row >= numberOfRows) {
        auto lastEntry =  [self objectAtIndex:numberOfRows-1];
        if (!playlist->moveSongs(draggedSongs, lastEntry, SongEntry())) {
            return NO;
        }
        
    } else {
        
        SongEntry aboveThis = [self objectAtIndex:row];
        SongEntry belowThis;
        
        if (row > 0) {
            belowThis = [self objectAtIndex:row-1];
        }
    
        if (!playlist->moveSongs(draggedSongs, belowThis, aboveThis)) {
            return NO;
        }
    }
    
    [self.musicListView deselectAll:nil];
    [self reloadTableForced];
    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
    NSArray *sortDescriptors = [musicListView sortDescriptors];
    if ([sortDescriptors count] == 0 || (![[[sortDescriptors objectAtIndex:0] key] isEqualToString:@"position"] && ![[[sortDescriptors objectAtIndex:0] key] isEqualToString:@"relevance"])) {
        // only enable drop if sorted by position in playlist
        return 0;
    }
    
    auto playlist = IApp::instance()->selectedPlaylist();
    if (playlist->reorderable()) {
        [musicListView setDropRow:row dropOperation:NSTableViewDropAbove];
        return NSDragOperationMove;
    }
    return 0;
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    if (self.albumSplitView.positionOfDivider > 1.0f) {
        [self.imageBrowserView.window makeFirstResponder:self.imageBrowserView];
    } else {
        [self.musicListView.window makeFirstResponder:self.musicListView];
    }
    
#ifndef ENABLE_PADDLE
    [[[NSApp mainMenu] itemAtIndex:0] submenu].title = @"Gear Player";
#endif
}

- (void)volumeIncrement:(float)inc
{
    float value = self.volumeSlider.floatValue;
    value += inc * self.volumeSlider.maxValue;
    if (value < 0) {
        value = 0;
    } else if (value > self.volumeSlider.maxValue) {
        value = self.volumeSlider.maxValue;
    }

    AnimationForSlider *sliderAnimation = [[AnimationForSlider alloc] initWithDuration:0.2 animationCurve:NSAnimationEaseIn];
    [sliderAnimation setAnimationBlockingMode:NSAnimationNonblocking];
    [sliderAnimation setSlider:self.volumeSlider];
    [sliderAnimation setAnimateToValue:value];
    [sliderAnimation startAnimation];
    
    
    [self volumeChange:self.volumeSlider];
}

@end
