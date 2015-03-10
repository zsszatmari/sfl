//
//  SongActionMenu.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 24/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import "SongActionMenu.h"
#include "SongEntry.h"
#include "ISongIntent.h"
#include "ISongArray.h"
#include "StringUtility.h"
#import "AppDelegate.h"

using namespace Gear;

@interface FontProxy : NSObject<UITableViewDataSource>

- (id)initWithDatasource:(NSObject<UITableViewDataSource> *)original;

@end

@implementation SongActionMenu {
    vector<SongEntry> songs;
    SongIntents intents;
    std::function<void()> _alertOk;
}

+ (SongActionMenu *)sharedMenu
{
    static dispatch_once_t onceToken;
    static SongActionMenu *menu = nil;
    dispatch_once(&onceToken, ^{
        menu = [[SongActionMenu alloc] init];
    });
    return menu;
}

// intents must be saved for later use on player screen, because search results might change
+ (SongIntents)songIntentsForSong:(const SongEntry &)entry songArray:(const shared_ptr<ISongArray> &)songsArray
{
    SongIntents intents;
    if (!entry.song() || !songsArray) {
        // invalid, e.g. empty player
        return intents;
    }
    
    string hint;
    vector<SongEntry> songs;
    songs.clear();
    songs.push_back(entry);
    
    intents.intents = songsArray->songIntents(songs, hint);
    intents.submenus = songsArray->songIntentSubMenus(songs);
    return intents;
}

- (void)actionForSong:(const SongEntry &)entry songArray:(const shared_ptr<ISongArray> &)songsArray
{
    if (!entry.song()) {
        // invalid, e.g. empty player
        return;
    }
    
    [self actionForSong:entry songIntents:[[self class] songIntentsForSong:entry songArray:songsArray]];
}

- (void)actionForSong:(const Gear::SongEntry &)entry songIntents:(const SongIntents &)songIntents
{
    songs.clear();
    songs.push_back(entry);
    intents = songIntents;
    
    if (songIntents.intents.empty() && songIntents.submenus.empty()) {
        return;
    }
    
    UIActionSheet *options = [[UIActionSheet alloc]
                              initWithTitle:convertString(entry.song()->title())
                              delegate:self cancelButtonTitle:nil
                              destructiveButtonTitle:nil
                              otherButtonTitles: nil];
	options.actionSheetStyle = UIActionSheetStyleBlackOpaque;
	
    [self addSubmenusTo:options];
    [self addIntents:songIntents.intents to:options];
    
    [options addButtonWithTitle:@"Cancel"];
    [options setCancelButtonIndex:options.numberOfButtons - 1];
    
	// maybe called by the player
	[options showInView: [UIApplication.sharedApplication.keyWindow.subviews lastObject]];
    //    [options showInView:self.view.superview];
}


- (void)addSubmenusTo:(UIActionSheet*) options
{
    for (auto it = intents.submenus.cbegin(); it != intents.submenus.cend(); it++) {
        NSString *text = convertString((*it).first);
        [options addButtonWithTitle:text];
    }
}


- (void)addIntents:(vector<shared_ptr<ISongIntent>>) items to:(UIActionSheet*)options
{
    intents.intents.clear();
    
    for (auto it = items.cbegin(); it != items.cend(); it++) {
        // bad idea, end is handled differently by table....
        //NSString *text = [NSString stringWithFormat:@"  %@  ",convertString((*it)->menuText())];
        NSString *text = convertString((*it)->menuText());
#if TARGET_IPHONE_SIMULATOR
        // reviewer does not like offline mode because we are somehow tricking the user (since it is not available in youtube)
        if ([text rangeOfString:@"offline"].location != NSNotFound) {
            continue;
        }
#endif
        
        //SLNSLOG(@"%@", text);
        
        intents.intents.push_back(*it);
        [options addButtonWithTitle:text];
        
        // only for the last item
        if (it == items.cend() -1) {
            auto range1 = [text rangeOfString:@"Remove" options:NSCaseInsensitiveSearch];
            auto range2 = [text rangeOfString:@"Delete" options:NSCaseInsensitiveSearch];
            //auto range3 = [text rangeOfString:@"Back" options:NSCaseInsensitiveSearch];
            
            if (range1.location != NSNotFound || range2.location != NSNotFound) {
                [options setDestructiveButtonIndex:options.numberOfButtons - 1];
            }
        }
    }
    
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex < intents.submenus.size()) {
        
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 0.5f * NSEC_PER_SEC);
        dispatch_after(popTime, dispatch_get_main_queue(), ^{
            
            auto submenu = intents.submenus.at(buttonIndex);
            auto subIntents = submenu.second;
            UIActionSheet *sheet = [[UIActionSheet alloc]
                                    initWithTitle:convertString(submenu.first)
                                    delegate:self cancelButtonTitle:nil
                                    destructiveButtonTitle:nil
                                    otherButtonTitles: nil];
            sheet.actionSheetStyle = UIActionSheetStyleBlackOpaque;
            
            [self addIntents:subIntents to:sheet];
            intents.submenus.clear();
            
            [sheet addButtonWithTitle:@"Back"];
            [sheet setCancelButtonIndex:sheet.numberOfButtons - 1];
            
            [sheet showInView: [UIApplication.sharedApplication.keyWindow.subviews lastObject]];
            dispatch_after(popTime, dispatch_get_main_queue(), ^(){
                
                [self willPresentActionSheet:sheet];
            });
        });
        
    }
    else if (buttonIndex < intents.submenus.size() + intents.intents.size()) {
        auto intent = intents.intents.at(buttonIndex - intents.submenus.size());
        if (buttonIndex == actionSheet.destructiveButtonIndex) {
            NSString *title = [actionSheet buttonTitleAtIndex:buttonIndex];
            NSString *message = @"Are you sure?";
            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:title message:message delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Yes", nil];
            _alertOk = [intent,self]{intent->apply(songs);};
            [alert show];
        } else {
            intent->apply(songs);
        }
    }
}

- (void)alertView:(UIAlertView *)alertView
clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex == 1 && _alertOk) {
        _alertOk();
    }
}

+ (void)setStyle:(UIView *)view
{
    UIFont *font = [UIFont systemFontOfSize:18.f];
    UILabel *label;
    if ([view isKindOfClass:[UIButton class]]) {
        label = [(UIButton *)view titleLabel];
    } else if ([view isKindOfClass:[UILabel class]]) {
        label = (UILabel *)view;
    }
    
    [label setFont:font];

    NSString *text = [label text];
    const int kLimit = 27;
    if ([text length] > kLimit) {
        text = [text stringByReplacingCharactersInRange:NSMakeRange(kLimit, [text length]-kLimit) withString:@"..."];
        [label setText:text];
    }
    
    /*
    NSString *text = [label text];
    if (text) {
        
        NSMutableParagraphStyle *style =  [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        style.alignment = NSTextAlignmentCenter;
        style.firstLineHeadIndent = 10.0f;
        style.headIndent = 10.0f;
        style.tailIndent = 10.0f;
        
        NSAttributedString *attrText = [[NSAttributedString alloc] initWithString:[label text] attributes:@{ NSParagraphStyleAttributeName : style}];
        [label setAttributedText:attrText];
    }*/
}

+ (void)recursiveSetFontIn:(UIView *)view level:(int)level
{
    
    for (id currentView in view.subviews) {
    
        if (![AppDelegate runningPreIOS7]) {
            if ([currentView isKindOfClass:[UIButton class]]) {
                [self setStyle:currentView];
            } else if ([currentView isKindOfClass:[UILabel class]] && level > 2) {
                [self setStyle:currentView];
            } else if ([currentView isKindOfClass:[UITableView class]]) {
                [self recursiveSetFontIn:currentView level:level+1];
                UITableView *tableView = currentView;
                // must be retained
                static FontProxy *_fontProxy;
                _fontProxy = [[FontProxy alloc] initWithDatasource:tableView.dataSource];
                tableView.dataSource = _fontProxy;
            } else {
                [self recursiveSetFontIn:currentView level:level+1];
            }
        } else {
            if ([currentView isKindOfClass:[UILabel class]]) {
                [self setStyle:currentView];
            }
        }
    }
}

- (void)willPresentActionSheet:(UIActionSheet *)actionSheet
{
    [[self class] recursiveSetFontIn:actionSheet level:0];
}

@end

@implementation FontProxy {
    NSObject<UITableViewDataSource> *_original;
}

- (id)initWithDatasource:(NSObject<UITableViewDataSource> *)original
{
    self = [super init];
    if (self) {
        _original = original;
    }
    return self;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [_original tableView:tableView numberOfRowsInSection:section];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [_original tableView:tableView cellForRowAtIndexPath:indexPath];
    [SongActionMenu recursiveSetFontIn:cell level:2];
    return cell;
}

@end

