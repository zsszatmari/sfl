//
//  ShortcutsController.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 1/30/13.
//
//

#import "MASShortcut.h"
#import "MASShortcutView.h"
#import "ShortcutsController.h"
#import "AppDelegate.h"
#import "MainWindowController.h"
#include "IApp.h"
#include "IPlayer.h"
#include "StringUtility.h"
#include "Painter.h"
#include "ThemeManager.h"
#include "ITheme.h"
#import "SongsTableView.h"

using namespace Gear;

@interface ShortcutsController () {
    NSMutableDictionary *shortcuts;
    NSMutableDictionary *shortcutIdsToKeys;
    NSDictionary *keysToViews;
}

- (void)hotkeyPressed:(int)keyId;
- (void)hotkeyReleased:(int)keyId;

@end

NSString * const kKeyShortcutThumbsUp = @"ShortcutThumbsUp";
NSString * const kKeyShortcutThumbsDown = @"ShortcutThumbsDown";
NSString * const kKeyShortcutPlayPause = @"ShortcutPlayPause";
NSString * const kKeyShortcutNext = @"ShortcutNext";
NSString * const kKeyShortcutPrevious = @"ShortcutPrevious";
NSString * const kKeyShortcutJumpToCurrent = @"ShortcutJumpToCurrent";

static const int kStartID = 1000;

@implementation ShortcutsController

static int getKeyId(EventRef theEvent)
{
    EventHotKeyID hkCom;
    
    OSStatus error = GetEventParameter(theEvent,kEventParamDirectObject,typeEventHotKeyID,NULL,
                                       sizeof(hkCom),NULL,&hkCom);
    
    
    if (error != 0) {
        return 0;
    }
    int keyId = hkCom.id;
    return keyId;
}

static OSStatus HotKeyHandlerPress(EventHandlerCallRef nextHandler,EventRef theEvent,void *userData)
{
    ShortcutsController *controller = (__bridge ShortcutsController *)(userData);
    
    int keyId = getKeyId(theEvent);
    if (keyId != 0) {
        [controller hotkeyPressed:keyId];
    }
    
    return 0;
}

static OSStatus HotKeyHandlerRelease(EventHandlerCallRef nextHandler,EventRef theEvent,void *userData)
{
    ShortcutsController *controller = (__bridge ShortcutsController *)(userData);
    
    int keyId = getKeyId(theEvent);
    if (keyId != 0) {
        [controller hotkeyReleased:keyId];
    }
    
    return 0;
}

- (void)awakeFromNib
{
    EventTypeSpec _eventType[2];
    _eventType[0].eventClass = kEventClassKeyboard;
    _eventType[0].eventKind = kEventHotKeyPressed;
    _eventType[1].eventClass = kEventClassKeyboard;
    _eventType[1].eventKind = kEventHotKeyReleased;
    InstallApplicationEventHandler(&HotKeyHandlerPress,1,_eventType,(__bridge void *)self,NULL);
    InstallApplicationEventHandler(&HotKeyHandlerRelease,1,_eventType+1,(__bridge void *)self,NULL);
    
    
    shortcuts = [[NSMutableDictionary alloc] init];
    shortcutIdsToKeys = [[NSMutableDictionary alloc] init];
    
    keysToViews = @{kKeyShortcutThumbsUp:self.thumbsUpView,
                   kKeyShortcutThumbsDown:self.thumbsDownView,
                   kKeyShortcutPlayPause:self.playPauseView,
                   kKeyShortcutNext:self.nextView,
                   kKeyShortcutPrevious:self.previousView,
                   kKeyShortcutJumpToCurrent:self.jumpToCurrentView};
    [keysToViews enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        [self setShortcutKey:key forView:obj];
    }];
    
    [self setupHandlers];
}

- (void)setShortcutKey:(NSString *)shortcutKey forView:(MASShortcutView *)view
{
    MASShortcut *shortcut = nil;
    id encodedShortcut = [[NSUserDefaults standardUserDefaults] objectForKey:shortcutKey];
    if ([encodedShortcut isKindOfClass:[NSData class]]) {
        shortcut = [MASShortcut shortcutWithData:encodedShortcut];
    }
        
    void(^shortcutChange)(id) = ^(id sender){
        [self saveValues];
        [self setupHandlers];
    };
    
    view.shortcutValue = shortcut;
    view.shortcutValueChange = shortcutChange;
}


- (void)setShortcut:(MASShortcutView *)shortcutView forKey:(NSString *)key
{
    MASShortcut *value = shortcutView.enabled ? shortcutView.shortcutValue : nil;
    
    if (value == nil) {
        [[NSUserDefaults standardUserDefaults] setObject:@0 forKey:key];
    } else {
        [[NSUserDefaults standardUserDefaults] setObject:[value data] forKey:key];
    }
}

- (void)saveValues
{
    [keysToViews enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        [self setShortcut:obj forKey:key];
    }];
}

- (void)setupHandlers
{
    for (NSValue *shortcut in [shortcuts allKeys]) {
        EventHotKeyRef ref;
        [shortcut getValue:&ref];
        UnregisterEventHotKey(ref);
    }
    [shortcuts removeAllObjects];
    [shortcutIdsToKeys removeAllObjects];
    
    NSArray *keys = [keysToViews allKeys];
    int i = kStartID;
    for (NSString *key in keys) {
        
        MASShortcutView *view = [keysToViews objectForKey:key];
        MASShortcut *shortcut = view.enabled ? view.shortcutValue : nil;
        
        if (shortcut == nil) {
            // no shortcut!
            continue;
        }
        
        EventHotKeyID hotKeyID;
        
        hotKeyID.signature='h';
        hotKeyID.id = i;
        
        UInt32 keyCode = shortcut.carbonKeyCode;
        UInt32 modifier = shortcut.carbonFlags;
        EventHotKeyRef ref;
        RegisterEventHotKey(keyCode, modifier, hotKeyID, GetEventDispatcherTarget(), 0, &ref);
        
        [shortcuts setObject:key forKey:[NSValue valueWithBytes:&ref objCType:@encode(EventHotKeyRef)]];
        [shortcutIdsToKeys setObject:key forKey:@(i)];
        
        i++;
    }
}

- (void)setRatingForCurrentSong:(int32_t)rating
{
    auto song = IApp::instance()->player()->songEntryConnector().value().song();
    if (song) {
        IApp::instance()->player()->rate(song, rating);
        
        if (mountainLionOrBetter()) {
            NSUserNotification *notification = [[NSUserNotification alloc] init];
            notification.subtitle = Gear::convertString(song->artist());
            notification.title = Gear::convertString(song->title());
            
            if (rating == 5) {
                notification.informativeText = @"Thumbs Up!";
            } else if (rating == 1) {
                notification.informativeText = @"Thumbs Down";
            }
            
            if ([notification respondsToSelector:@selector(setContentImage:)]) {
                notification.contentImage = Gui::Painter::convertImage(*IApp::instance()->themeManager()->current()->ratingIcon(rating));;
            }
        
            notification.userInfo = @{@"forceShow":@YES};
            [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
            [[[[AppDelegate sharedDelegate] mainWindowController] musicListView] reloadData];
        }
    }
}

- (void)hotkeyPressed:(int)keyId
{
    NSString *key = [shortcutIdsToKeys objectForKey:@(keyId)];
    
    if ([key isEqualToString:kKeyShortcutThumbsUp]) {
        [self setRatingForCurrentSong:5];
    } else if ([key isEqualToString:kKeyShortcutThumbsDown]) {
        [self setRatingForCurrentSong:1];
    } else if ([key isEqualToString:kKeyShortcutPlayPause]) {
        [[AppDelegate sharedDelegate].mainWindowController togglePlay:nil];
    } else if ([key isEqualToString:kKeyShortcutNext]) {
        [[AppDelegate sharedDelegate].mainWindowController next:nil];
    } else if ([key isEqualToString:kKeyShortcutPrevious]) {
        [[AppDelegate sharedDelegate].mainWindowController prev:nil];
    } else if ([key isEqualToString:kKeyShortcutJumpToCurrent]) {
        [[AppDelegate sharedDelegate].mainWindowController jumpToSong:nil];
    }
}

- (void)hotkeyReleased:(int)keyId
{
    // don't care
}


@end
