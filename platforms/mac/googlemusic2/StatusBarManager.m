//
//  StatusBarManager.m
//  G-Ear
//
//  Created by Zsolt Szatmari on 6/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "StatusBarManager.h"
#import "AppDelegate.h"
#import "NSImage+Tinting.h"

NSString *kUserDefaultsStatusBarKey = @"ShowIconsInStatusBar";

@interface StatusBarManager() {
    NSStatusItem *prev;
    NSStatusItem *playpause;
    NSStatusItem *next;
    NSStatusItem *songInfo;
    NSStatusItem *appIcon;
    
    BOOL playing;
    
    NSString *tooltipText;
    NSString *song;
    NSString *artist;
    BOOL songInfoChange;
    
    BOOL prevStatusBarModeSetting;
    BOOL appIconNeeded;
}

@end

@implementation StatusBarManager 

@synthesize delegate;

- (void)setImage:(NSString *)imageName forItem:(NSStatusItem *)statusItem
{
    [self setImage:imageName forItem:statusItem reduceSize:11.0f];
}

- (void)setImage:(NSString *)imageName forItem:(NSStatusItem *)statusItem reduceSize:(CGFloat)reduce
{
    [statusItem setHighlightMode:YES];
    
    // #define NSAppKitVersionNumber10_9 1265
    BOOL yosemiteOrNewer = floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_9;
    
    if (![imageName isEqualToString:@"miniicon"]) {
        NSImage *image = [NSImage imageNamed:imageName];
        if (yosemiteOrNewer) {
            [image setTemplate:YES];
        }
        [statusItem setImage:image];
    } else {
        
        NSImage *image;
        //if (!isPause && NO) {
        //    image = [[[NSImage imageNamed:imageName] copy] imageTintedWithColor:[NSColor blackColor]];
        //} else {
            image = [NSImage imageNamed:imageName];
        //}
        
        if (yosemiteOrNewer) {
            [image setTemplate:YES];
        }
        // standard status bar thickness is 22
        CGFloat height = [[statusItem statusBar] thickness] - reduce;
        [image setSize:CGSizeMake([image size].width * height / [image size].height, height)];
        
        [statusItem setImage:image];
    }
}

- (BOOL)appIconNeeded
{
    return appIconNeeded;
}

- (void)setupStatusBar
{
    prevStatusBarModeSetting = [[NSUserDefaults standardUserDefaults] boolForKey:@"statusbarMode"];
    appIconNeeded = prevStatusBarModeSetting;
    
    void (^showOrHide)(NSNotification *) = ^(NSNotification *note) {
    
        BOOL statusBarModeSetting = [[NSUserDefaults standardUserDefaults] boolForKey:@"statusbarMode"];
        if (statusBarModeSetting != prevStatusBarModeSetting) {
            if (!statusBarModeSetting) {
                
                [AppDelegate showInDock];
                
                appIconNeeded = NO;
            } else {
                appIconNeeded = YES;
                
                NSAlert *alert = [[NSAlert alloc] init];
                [alert setMessageText:@"Dock Icon"];
                [alert setInformativeText:@"Dock icon status change will take effect on next launch of G-Ear."];
                [alert setAlertStyle:NSWarningAlertStyle];
                [alert runModal];                
            }
            prevStatusBarModeSetting = statusBarModeSetting;
        }
        
        [self showStatusBar];
        
        [self updateSongDisplay];
    };
    
    [[NSNotificationCenter defaultCenter] addObserverForName:NSUserDefaultsDidChangeNotification object:nil queue:nil usingBlock:showOrHide];
    
    showOrHide(nil);
}

- (BOOL)statusBarNeeded
{
    NSUserDefaultsController *userDefaults = [NSUserDefaultsController sharedUserDefaultsController];
    NSNumber *statusBarNumber = [[userDefaults values] valueForKey:kUserDefaultsStatusBarKey];
    BOOL statusBar = [statusBarNumber boolValue];
    return statusBar;
}

- (void)updateSongDisplay
{
    NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
    
    NSUserDefaultsController *userDefaults = [NSUserDefaultsController sharedUserDefaultsController];
    NSNumber *songInfoNumber = [[userDefaults values] valueForKey:@"ShowSongInfoInStatusBar"];
    BOOL songInfoNeeded = [songInfoNumber boolValue];
    
    if (songInfo != nil && !songInfoNeeded) {
        [statusBar removeStatusItem:songInfo];
        songInfo = nil;
        return;
    }
    
    if (songInfoNeeded) {
        
        if (songInfo != nil) {
            
            if (!songInfoChange) {
                return;
            }
            
            [statusBar removeStatusItem:songInfo];
        }
        if ([self statusBarNeeded]) {
            [self hideStatusBar];
            [self doShowStatusBar];
        }
        songInfoChange = NO;
        
        CGFloat offset = 2;
        CGFloat thickness = [statusBar thickness];
        CGFloat rowHeight = (thickness -offset) /2;
        //NSFont *font = [NSFont fontWithName:@"Helvetica" size:rowHeight];
        
        CGFloat extraMinus = 1;
        NSFont *font = [NSFont systemFontOfSize:rowHeight-extraMinus];
        
        NSDictionary *textAttributes = @{NSFontAttributeName:font};
        CGFloat proposedWidth = [song sizeWithAttributes:textAttributes].width;
        CGFloat width = proposedWidth;
        proposedWidth = [artist sizeWithAttributes:textAttributes].width;
        if (proposedWidth > width) {
            width = proposedWidth;
        }
        if (width > 0) {
            width += 10;
        }
        
        songInfo = [statusBar statusItemWithLength:width];
        
        NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, width, thickness)];
        
        if (song == nil) {
            song = @"";
        }
        if (artist == nil) {
            artist = @"";
        }
        
        NSButton *topRow = [[NSButton alloc] initWithFrame:NSMakeRect(0, rowHeight + extraMinus, width, rowHeight+offset)];
        [topRow setBordered:NO];
        [topRow setAlignment:NSCenterTextAlignment];
        [topRow setTitle:song];
        //[topRow setSelectable:NO];
        [topRow setFont:font];
        [topRow setAction:@selector(statusTextClicked:)];
        [topRow setTarget:self.delegate];
        //[topRow setBackgroundColor:[NSColor redColor]];
        
        NSButton *bottomRow = [[NSButton alloc] initWithFrame:NSMakeRect(0, extraMinus, width, rowHeight+offset)];
        [bottomRow setBordered:NO];
        [bottomRow setAlignment:NSCenterTextAlignment];
        [bottomRow setTitle:artist];
        [bottomRow setFont:font];
        [bottomRow setAction:@selector(statusTextClicked:)];
        [bottomRow setTarget:self.delegate];
        
        [view addSubview:topRow];
        [view addSubview:bottomRow];
        songInfo.view = view;
    }
}

- (void)showStatusBar
{
    [self doShowStatusBar];
    [self updateSongDisplay];
}

- (void)doShowStatusBar
{
    BOOL showNeeded = NO;
    BOOL controlsPresent = (next != nil && playpause != nil && prev != nil);
    if ((!controlsPresent && [self statusBarNeeded]) || (controlsPresent && ![self statusBarNeeded])) {
        showNeeded = YES;
    }
    if ((appIcon != nil && ![self appIconNeeded]) || (appIcon == nil && [self appIconNeeded])) {
        showNeeded = YES;
    }
    if (!showNeeded) {
        return;
    }
    
    NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
    
    if (songInfo != nil) {
        [statusBar removeStatusItem:songInfo];
        songInfo = nil;
    }
    [self hideStatusBar];
    
    CGFloat size = [statusBar thickness];
    
    // order is reversed
    if ([self statusBarNeeded]) {
    
        next = [statusBar statusItemWithLength:size];
        [self setImage:@"nextmini" forItem:next];
        playpause = [statusBar statusItemWithLength:size];
        [self setPlaying:playing];    
        prev = [statusBar statusItemWithLength:size];
        [self setImage:@"prevmini" forItem:prev];
    }
    if ([self appIconNeeded]) {
        appIcon = [statusBar statusItemWithLength:size];
        [self setImage:@"miniicon" forItem:appIcon reduceSize:4];
    }

    [prev setTarget:self.delegate];
    [prev setAction:@selector(prev:)];
    
    [playpause setTarget:self.delegate];
    [playpause setAction:@selector(togglePlay:)];

    [next setTarget:self.delegate];
    [next setAction:@selector(next:)];
    
    [appIcon setTarget:self];
    [appIcon setAction:@selector(appIconClicked:)];

    [self refreshTooltip];
}

- (void)appIconClicked:(id)sender
{
    static NSDate *lastClick = nil;
    NSDate *now = [NSDate date];
    BOOL showMenu = NO;
    if (lastClick != nil && [now timeIntervalSinceDate:lastClick] < 10) {
        showMenu = YES;
    }
    lastClick = now;
    
    NSWindow *window = [AppDelegate sharedDelegate].window;
    [window orderFrontRegardless];
    
    if (showMenu) {
        NSMenu *menu = [[NSMenu alloc] init];
        NSMenuItem *item = [menu addItemWithTitle:@"Quit G-Ear" action:@selector(terminate:) keyEquivalent:@""];
        [item setTarget:[NSApplication sharedApplication]];
        [appIcon popUpStatusItemMenu:menu];
    }
}

- (void)setPlaying:(BOOL)aPlaying
{
    playing = aPlaying;
    [self setImage:(playing ? @"pausemini" : @"playmini") forItem:playpause];
}

- (void)hideStatusBar
{
    NSStatusBar *statusBar = [NSStatusBar systemStatusBar];

    if (prev != nil) {
        [statusBar removeStatusItem:prev];
        prev = nil;
    }
    if (playpause != nil) {
        [statusBar removeStatusItem:playpause];
        playpause = nil;
    }
    if (next != nil) {
        [statusBar removeStatusItem:next];
        next = nil;
    }
    if (appIcon != nil) {
        [statusBar removeStatusItem:appIcon];
        appIcon = nil;
    }
}

/*- (NSString *)padString:(NSString *)string toLengthOf:(NSString *)lengthOf
{
    NSUInteger length = [lengthOf length];
    if (length <= [string length]) {
        return string;
    }
    NSString *newString = [@"" stringByPaddingToLength:(length - [string length]) /2 withString:@" " startingAtIndex:0];
    newString = [newString stringByAppendingString:string];
    newString = [newString stringByPaddingToLength:length withString:@" " startingAtIndex:0];
    return newString;
}*/

- (void)setDisplayedAlbumArtist:(NSString *)aAlbumArtist song:(NSString *)aSong artist:(NSString *)aArtist
{
    if (aAlbumArtist == nil) {
        tooltipText = aSong;
    } else if (song == nil) {
        tooltipText = aAlbumArtist;
    } else {
        //albumArtist = [self padString:albumArtist toLengthOf:song];
        //song        = [self padString:song toLengthOf:albumArtist];
        tooltipText = [NSString stringWithFormat:@"%@\n%@",aAlbumArtist,aSong];
    }
    
    if (song == nil) {
        song = @"";
    }
    if (artist == nil) {
        artist = @"";
    }
    if (![song isEqualToString:aSong] || ![artist isEqualToString:aArtist]) {
        songInfoChange = YES;
        song = aSong;
        artist = aArtist;
        
        [self updateSongDisplay];
    }
    // this must be after updateSongDisplay, because buttons might have been recreated meanwhile
    [self refreshTooltip];
}

- (void)refreshTooltip
{
    [self refreshTooltipFor:prev];
    [self refreshTooltipFor:playpause];
    [self refreshTooltipFor:next];
    [self refreshTooltipFor:appIcon];
}

- (void)refreshTooltipFor:(NSStatusItem *)item
{
    if (tooltipText == nil) {
        tooltipText = @"";
    }
    [item setToolTip:tooltipText];
}

@end
