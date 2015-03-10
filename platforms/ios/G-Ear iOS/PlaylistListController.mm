//
//  playlistListController.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "PlaylistListController.h"

#import "SongsViewController.h"
#import "PlaylistCell.h"
#include "IApp.h"

#include "ThemeManager.h"
#include "ITheme.h"
#include "Painter.h"
#include "IPaintable.h"
#import "AppDelegate.h"
#import "SongsViewController.h"
#import "Debug.h"
#include "StringUtility.h"
#import "AnimationUtility.h"
#include "IPlayer.h"
#include "IPlaylistIntent.h"
#import "SongListController.h"
#import "FastScrollIndex.h"
#include "ISession.h"

using namespace Gear;

@interface PlaylistListController () {
    SignalConnection playlistCategoryConnection;
    SignalConnection playlistConnection;
    vector<shared_ptr<IPlaylist>> playlists;
    vector<shared_ptr<IPlaylistIntent>> intents;
    int categoryTag;
    int categoryLevel;
    UISwipeGestureRecognizer *swipe;
    CGFloat _cellHeight;
    FastScrollIndex *fastScrollIndex;
    
    uint32_t _currentView;
    std::map<uint32_t,int> _viewOffsets;
    BOOL scrollIndexNeeded;
    id fastScrollObserver;
    
    std::function<void()> _alertOk;
    
    CAGradientLayer *fadeMask;
}

@end

@implementation PlaylistListController

// this is clearly a hack
static CGFloat artistPosition = 0;

@synthesize tableView;

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) {
        fastScrollIndex = [[FastScrollIndex alloc] init];
    }
    return self;
}

- (FastScrollIndex *)fastScrollIndex
{
    return fastScrollIndex;
}

- (void)jumpToCurrent
{
    if (categoryTag == kArtistsTag) {
        CGFloat pos = MAX(0,MIN(artistPosition, self.tableView.contentSize.height - self.tableView.bounds.size.height));
        self.tableView.contentOffset = CGPointMake(0,pos);
        return;
    }
    
    {
        auto it = _viewOffsets.find(_currentView);
        if (it != _viewOffsets.end()) {
            int i = it->second;
            i = MAX(0,MIN(i, self.tableView.contentSize.height - self.tableView.bounds.size.height));
            self.tableView.contentOffset = CGPointMake(0, i);
            return;
        }
    }
    
    auto current = IApp::instance()->player()->playlistCurrentlyPlaying();
    int i = 0;
    for (auto it = playlists.begin() ; it != playlists.end() ; ++it) {
        if (*it == current) {
            [self.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:i inSection:0] atScrollPosition:UITableViewScrollPositionMiddle animated:NO];
            break;
        }
        ++i;
    }
}

- (void)viewDidLoad
{
    categoryTag = -1;
    [super viewDidLoad];
	if (! [AppDelegate runningPreI5]) {
 		self.tableView.frame = CGRectMake(0, 0, 320, 468);
    }
    
    UILongPressGestureRecognizer *lpgr = [[UILongPressGestureRecognizer alloc]
                                          initWithTarget:self action:@selector(handleLongPress:)];
    lpgr.minimumPressDuration = 0.3f; //seconds
    [self.tableView addGestureRecognizer:lpgr];
    
    __weak PlaylistListController *wself = self;
    playlistCategoryConnection = IApp::instance()->selectedPlaylistConnector().connect([wself](const pair<PlaylistCategory,shared_ptr<IPlaylist>> &p){
        
        PlaylistListController *s = wself;
        if (!s || s.preventUpdates) {
            return;
        }
        const PlaylistCategory &rhs = p.first;
        PLNSLOG("selected connector: %@ %d", convertString(rhs.title()), rhs.tag());
        if (rhs.singularPlaylist()) {
            return;
        }
        
        BOOL isSwitching = (s->categoryTag != rhs.tag()) || (s->categoryLevel != rhs.level());
        if (isSwitching && s->categoryTag != -1) {
            return;
        }

        s->categoryLevel = rhs.level();
        s->categoryTag = rhs.tag();
        if (isSwitching) {
            s->_viewOffsets[s->_currentView] = s.tableView.contentOffset.y;
        }
        s->_currentView = rhs.tag() | (rhs.level() << 16);
        
        s->scrollIndexNeeded = ((rhs.tag() == kArtistsTag) || (rhs.tag() == kAlbumArtistsTag)) && (rhs.level() == 0);
        
        switch(rhs.presentation()) {
            case Gear::PlaylistCategory::Presentation::Normal:
                s->_cellHeight = 44;
                break;
            case Gear::PlaylistCategory::Presentation::Albums:
                s->_cellHeight = 96;
                // 90 is the album size
                break;
        }
        [s tableView].rowHeight = s->_cellHeight;
        
        s->playlists = rhs.playlists();
        [s.tableView reloadData];
        
        // jump
        [s jumpToCurrent];
    });
    
    swipe = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipe:)];
    swipe.direction = UISwipeGestureRecognizerDirectionRight;
    [self.tableView addGestureRecognizer:swipe];
}

- (void)setParentView:(UIView *)parent
{
#ifdef DEBUG
    //NSLog(@"song list bounds: %@", NSStringFromCGRect(parent.bounds));
#endif
    self.view.frame = parent.bounds;
    tableView.frame = parent.bounds;
    
    tableView.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    fadeMask.frame = parent.bounds;
}

- (void)applyTheme
{
    auto theme = IApp::instance()->themeManager()->current();
    auto color = dynamic_pointer_cast<Gui::Color>(theme->listBackground());
    if (color) {
        UIColor *fade_in = [UIColor colorWithRed:color->red() green:color->green() blue:color->blue() alpha: 1.];
        UIColor *fade_out = [UIColor colorWithRed:color->red() green:color->green() blue:color->blue() alpha: .0];
        fadeMask.colors = @[(id) fade_in.CGColor, (id) fade_out.CGColor, (id) fade_out.CGColor, (id) fade_in.CGColor];
    }
    ((GearImageView*)self.view).image = theme->listBackground();
}

- (IBAction)swipe:(id)sender
{
    IApp::instance()->goBack();
}


- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
	[self.tableView deselectRowAtIndexPath: [self.tableView indexPathForSelectedRow] animated: YES];
    auto theme = IApp::instance()->themeManager()->current();
    
    self.tableView.backgroundColor = [UIColor clearColor];
//    [self.tableView setSeparatorStyle:UITableViewCellSeparatorStyleNone];

	if ([self.view.layer.sublayers count] < 2) {
        
		fadeMask = [CAGradientLayer layer];

        fadeMask.locations = [AppDelegate runningPreI5] ? @[@.0, @.06, @.92, @.98] : @[@.0, @.06, @.925, @.985];
//        fade_mask.frame = self.view.bounds;
		fadeMask.frame = self.tableView.frame;

		[self.view.layer addSublayer: fadeMask];
	}
    [self applyTheme];

    [self jumpToCurrent];
    
    __weak PlaylistListController *wself = self;
    fastScrollObserver = [[NSNotificationCenter defaultCenter] addObserverForName:kNotificationFastScroll object:fastScrollIndex queue:nil usingBlock:^(NSNotification *note) {
        
        PlaylistListController *s = wself;
        if (!s) {
            return;
        }
        NSNumber *obj = [[note userInfo] objectForKey:@"position"];
        
        int index = [obj unsignedIntegerValue];
        auto size = s->playlists.size();
        if (index >= size) {
            index = size-1;
        }
        if (index < 0) {
            index = 0;
        }
        
        [s.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:index inSection:0] atScrollPosition:UITableViewScrollPositionMiddle animated:NO];
    }];
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
    [super willRotateToInterfaceOrientation:toInterfaceOrientation duration:duration];
    
    self.tableView.frame = self.tableView.superview.bounds;
    self.tableView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    [super didRotateFromInterfaceOrientation:fromInterfaceOrientation];
    fadeMask.frame = self.tableView.frame;
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void) viewWillDisappear: (BOOL) animated
{
    _viewOffsets[_currentView] = self.tableView.contentOffset.y;
    [[NSNotificationCenter defaultCenter] removeObserver:fastScrollObserver];
    
	[_animator removeAllBehaviors];
	[super viewWillDisappear: animated];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if (scrollIndexNeeded) {
        [fastScrollIndex setPlaylists:playlists];
    } else {
        [fastScrollIndex setPlaylists:vector<shared_ptr<IPlaylist>>()];
    }
    return playlists.size();
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    
    shared_ptr<IPlaylist> entry = playlists.at([indexPath row]);
    
    NSString* cellId;
    if (!entry) {
        return [[PlaylistCell alloc] init];
    }
    
    //if (entry->image() == nullptr) {
    //    cellId = @"PlaylistCell";
    //} else {
        cellId = @"PlaylistImageCell";
    //}
    
    PlaylistCell *cell = [self.tableView
                          dequeueReusableCellWithIdentifier:cellId];
    CGRect frame = cell.frame;
    frame.size.height = _cellHeight;
    cell.frame = frame;
    
    auto theme = IApp::instance()->themeManager()->current();
    cell.nameAttributes = theme->listTitleText();
    cell.separator.image = theme->listSeparator();
    cell.separatorThickness = theme->listSeparatorThickness();
    cell.backgroundColor = [UIColor clearColor];

	UIView *bgv = [[UIView alloc] init];
	bgv.backgroundColor = Gui::Painter::convertColor(IApp::instance()->themeManager()->current()->selectedTextBackground());
	cell.selectedBackgroundView = bgv;

    [cell setPlaylist:entry];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (categoryTag == kArtistsTag) {
        artistPosition = self.tableView.contentOffset.y;
    }
    
    shared_ptr<IPlaylist> entry = playlists.at([indexPath row]);
    IApp::instance()->userSelectedPlaylist(entry);
}

- (void)returnFromPlaylist
{
    if ([[self.navigationController topViewController] isKindOfClass:[PlaylistListController class]]) {
        return;
    }
    SongsViewController *parentController = (SongsViewController *)self.navigationController.parentViewController;
	parentController.transition_dir = TRANSITION_FROM_RIGHT;
    [parentController performTransition:^{
        [[self navigationController] popToViewController:self animated:YES];
    }];
}

/*
- (void)drawRect:(CGRect)rect
{
    if (_background) {
        Gui::Painter::paint(_background, true);
    }
}
*/
- (void)viewDidUnload
{
    [self setTableView:nil];
    [super viewDidUnload];
}

- (void)handleLongPress:(UILongPressGestureRecognizer *)gestureRecognizer
{
    shared_ptr<IPlaylist> playlist;
	if (gestureRecognizer) {
        
		if (gestureRecognizer.state != UIGestureRecognizerStateBegan){
			return;
		}
        
		CGPoint p = [gestureRecognizer locationInView:self.tableView];
        
		NSIndexPath *indexPath = [self.tableView indexPathForRowAtPoint:p];
        
		if (indexPath == nil) {
			return;
		}
        
		playlist = playlists.at([indexPath row]);
	} else {
        return;
	}
    
    vector<shared_ptr<IPlaylistIntent>> playlistIntents = playlist->playlistIntents();
    if (playlistIntents.empty()) {
        return;
    }
    
    UIActionSheet *options = [[UIActionSheet alloc]
                              initWithTitle:convertString(playlist->name())
                              delegate:self cancelButtonTitle:nil
                              destructiveButtonTitle:nil
                              otherButtonTitles: nil];
	options.actionSheetStyle = UIActionSheetStyleBlackOpaque;
	
    [self addIntents:playlistIntents to:options];
    
    [options addButtonWithTitle:@"Cancel"];
    [options setCancelButtonIndex:options.numberOfButtons - 1];
    
	// maybe called by the player
	[options showInView: [UIApplication.sharedApplication.keyWindow.subviews lastObject]];
}

- (void)addIntents:(vector<shared_ptr<IPlaylistIntent>>)items to:(UIActionSheet*)options
{
    intents.clear();
    
    for (auto it = items.cbegin(); it != items.cend(); it++) {
        NSString *text = convertString((*it)->menuText());
        
        SLNSLOG(@"%@", text);
        
        intents.push_back(*it);
        [options addButtonWithTitle:text];
        
        auto range1 = [text rangeOfString:@"Remove" options:NSCaseInsensitiveSearch];
        auto range2 = [text rangeOfString:@"Delete" options:NSCaseInsensitiveSearch];
        
        if (range1.location != NSNotFound || range2.location != NSNotFound) {
            [options setDestructiveButtonIndex:options.numberOfButtons - 1];
        }
    }
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex < intents.size()) {
        auto intent = intents.at(buttonIndex);
        if (buttonIndex == actionSheet.destructiveButtonIndex) {
            NSString *title = [actionSheet buttonTitleAtIndex:buttonIndex];
            NSString *message = @"Are you sure?";
            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:title message:message delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"Yes", nil];
            _alertOk = [intent]{intent->apply();};
            [alert show];
        } else {
            intent->apply();
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

@end
