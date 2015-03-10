//
//  SongListController.m
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/11/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "SongListController.h"
#include "SignalConnection.h"
#include "IApp.h"
#include "StringUtility.h"
#import "SongCell.h"
#import "SongsViewController.h"
#include "IPlayer.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "ISongArray.h"
#include "Painter.h"
#include "SongPredicate.h"
#include "ISongIntent.h"
#import "AppDelegate.h"
#import "Debug.h"
#include "ISession.h"
#import "SongsViewController.h"
#import "FastScrollIndex.h"
#import "SongActionMenu.h"
#include "ValidPtr.h"
#include "Chain.h"
#import "HelpOverlay.h"
#include "IPlaylist.h"

using namespace Gear;

@interface SongListController () {
    
    SignalConnection playlistConnection;
    SignalConnection updateConnection;
    
    // songArray must be retained for continous playback and refreshing, e.g. radios would not work without this
    shared_ptr<ISongArray> _songArray;
    ValidPtr<const Chain<SongEntry>> _songs;
    
    id fastScrollObserver;
    BOOL scrollIndexNeeded;
    FastScrollIndex *fastScrollIndex;
    int fastScrollIndexCounter;
    
    CAGradientLayer *fadeMask;
}
@end

@implementation SongListController

@synthesize tableView;

- (FastScrollIndex *)fastScrollIndex
{
    return fastScrollIndex;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
	[self.tableView deselectRowAtIndexPath: [self.tableView indexPathForSelectedRow] animated: YES];
    auto theme = IApp::instance()->themeManager()->current();
    ((GearImageView*)self.view).image = theme->listBackground();
	//[self setAnimator7];
    
    __weak SongListController *wself = self;
    fastScrollObserver = [[NSNotificationCenter defaultCenter] addObserverForName:kNotificationFastScroll object:fastScrollIndex queue:nil usingBlock:^(NSNotification *note) {
        
        SongListController *s = wself;
        if (!s) {
            return;
        }
        NSNumber *obj = [[note userInfo] objectForKey:@"position"];
        
        int index = [obj unsignedIntegerValue];
        auto size = _songs->size();
        if (index >= size) {
            index = size-1;
        }
        if (index < 0) {
            index = 0;
        }
        
        [s.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:index inSection:0] atScrollPosition:UITableViewScrollPositionMiddle animated:NO];
    }];
    
    // e.g. coming back from player screen
    //[self jumpToCurrent];
    
    scrollIndexNeeded = !IApp::instance()->selectedPlaylist()->songArray()->orderedArray();
    // this is bad here, visibility will be controlled by the scrollindex itself
    //[[SongsViewController sharedController] setScrollIndexVisible:scrollIndexNeeded];
    if (!scrollIndexNeeded) {
        [fastScrollIndex setAlpha:0.0f];
    }

    // needed to redisplay song index when returning from playlist list...
    [self reloadData];
    // don't jump to current on refreshes, that would be annoying. jump only now
    [self jumpToCurrent];
    
#ifdef PINCH_IS_HELP
    static UIPinchGestureRecognizer *pinch;
    pinch = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(pinchGesture:)];
    [self.tableView addGestureRecognizer:pinch];
#endif
}

#ifdef PINCH_IS_HELP
- (void)pinchGesture:(UIGestureRecognizer *)sender
{
    if ([self swipeBackPossible] && [sender state] == UIGestureRecognizerStateEnded) {
        [HelpOverlay showForced:@"songlist"];
    }

}
#endif

- (void)didMoveToParentViewController:(UIViewController *)parent
{
    if (parent != nil) {
        
    }
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];

    
    if ([self swipeBackPossible]) {
        [HelpOverlay showIfNecessary:@"songlist" fromController:self];
    }
}

- (void) viewWillDisappear: (BOOL) animated
{
    scrollIndexNeeded = NO;
    [[NSNotificationCenter defaultCenter] removeObserver:fastScrollObserver];
    
	[_animator removeAllBehaviors];
	[super viewWillDisappear: animated];
}

//static int count = 0;

- (id)initWithCoder:(NSCoder *)aDecoder
{
    //NSLog(@"SONGLIST init %f", [NSDate timeIntervalSinceReferenceDate]);
    
    self = [super initWithCoder:aDecoder];
    if (self) {
        //SLNSLOG(@"songlist const %d", ++count);
        fastScrollIndex = [[FastScrollIndex alloc] init];
    }
    return self;
}

- (void)dealloc
{
    //SLNSLOG(@"songlist destr %d", --count);
}

- (void)viewDidLoad
{
    self.tableView.backgroundColor = [UIColor clearColor];

    if (! [AppDelegate runningPreI5]) {
		self.tableView.frame = CGRectMake(0, 0, 320, 468);
    }
    UILongPressGestureRecognizer *lpgr = [[UILongPressGestureRecognizer alloc]
                                          initWithTarget:self action:@selector(handleLongPress:)];
    lpgr.minimumPressDuration = 0.3f; //seconds
    [self.tableView addGestureRecognizer:lpgr];

    auto songsArray = [self songArray];

	__weak SongListController *wself = self;

    // needed on ios 7

    if ([self.tableView respondsToSelector:@selector(setSectionIndexBackgroundColor:)]) {
        self.tableView.sectionIndexBackgroundColor = [UIColor clearColor];
    }
    if ([self.tableView respondsToSelector:@selector(setSectionIndexColor:)]) {
        self.tableView.sectionIndexColor = [UIColor whiteColor];
        self.tableView.sectionIndexTrackingBackgroundColor = [UIColor clearColor];
    }

	((SongsViewController *) self.navigationController.parentViewController).tabBar.collection_view.userInteractionEnabled = YES;


	fadeMask = [CAGradientLayer layer];

    [self applyTheme];
//	fade_mask.locations = @[@.0, @.06, @    .92, @.98];
	fadeMask.locations = [AppDelegate runningPreI5] ? @[@.0, @.06, @.92, @.98] : @[@.0, @.06, @.925, @.985];
//        fade_mask.frame = self.view.bounds;
	fadeMask.frame = self.tableView.frame;

    [self.view.layer addSublayer: fadeMask];
    
    
	playlistConnection = IApp::instance()->selectedPlaylistConnector().connect([wself](const pair<PlaylistCategory,shared_ptr<IPlaylist>> &p){
        
        const shared_ptr<IPlaylist> &rhs = p.second;
        if (!rhs) {
            return;
        }
        
        SongListController *s = wself;
        if (s && ! s.preventUpdates) {
            
            if (s.playlist && s.playlist != IApp::instance()->selectedPlaylist()) {
                return;
            }
            s.playlist = IApp::instance()->selectedPlaylist();
            auto oldArray = s->_songArray;
            if (s.playlist) {
                s->_songArray = s.playlist->songArray();
            } else {
                s->_songArray = nullptr;
            }
            
            auto onUpdate = [wself]{
                
                SongListController *s = wself;
                if (s) {
                    [s reloadData];
                }
            };
            if (s->_songArray) {
                s->updateConnection = s->_songArray->updatedEvent().connect([onUpdate]{
                    // this delay is intentional
                    dispatch_async(dispatch_get_main_queue(), ^{
                        onUpdate();
                    });
                });
            }
            onUpdate();
        }
	});
}

- (void)setParentView:(UIView *)parent
{
#ifdef DEBUG
    //NSLog(@"song list bounds: %@", NSStringFromCGRect(parent.bounds));
#endif
    self.view.frame = parent.bounds;
    tableView.frame = parent.bounds;
    //tableView.backgroundColor = [UIColor greenColor];
    
    //tableView.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
    tableView.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    fadeMask.frame = parent.bounds;
}

- (void)applyTheme
{
    auto color = dynamic_pointer_cast<Gui::Color>(IApp::instance()->themeManager()->current()->listBackground());
    if (color) {
        UIColor *fade_in = [UIColor colorWithRed:color->red() green:color->green() blue:color->blue() alpha: 1.];
        UIColor *fade_out = [UIColor colorWithRed:color->red() green:color->green() blue:color->blue() alpha: .0];
        fadeMask.colors = @[(id) fade_in.CGColor, (id) fade_out.CGColor, (id) fade_out.CGColor, (id) fade_in.CGColor];
    }
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
    [super willRotateToInterfaceOrientation:toInterfaceOrientation duration:duration];
    
    //self.tableView.frame = self.tableView.superview.bounds;
    //self.tableView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    [super didRotateFromInterfaceOrientation:fromInterfaceOrientation];
    
    [CATransaction begin];
    [CATransaction setDisableActions: YES];
    fadeMask.frame = self.tableView.frame;
    [CATransaction commit];
    
    //NSLog(@"tableview frame: %@", NSStringFromCGRect(self.tableView.frame));
}

- (void)jumpToSong:(const SongEntry &)song
{
    if (song) {
#ifdef DEBUG
        //std::cout << "scrolling to: " << (std::string)song.song()->title() << std::endl;
#endif
        auto scrollIndex = _songs->indexOf(song);
        if (scrollIndex <= 6) {
            scrollIndex = 0;
        }
        if (scrollIndex != ISongArray::NotFound && scrollIndex < _songs->size()) {
            [self.tableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:scrollIndex inSection:0] atScrollPosition:UITableViewScrollPositionMiddle animated:NO];
        }
    }
}

- (void)jumpToCurrent
{
    auto playing = IApp::instance()->player()->songEntryConnector().value();
    [self jumpToSong:playing];
}

- (void)reloadData
{    
	SLNSLOG(@"reload");
    
    SongEntry center;
    NSArray *visible = [self.tableView indexPathsForVisibleRows];
    if ([visible count] > 0) {
        NSIndexPath *path = [visible objectAtIndex:[visible count]/2];
        auto index = path.row;
        if (index < _songs->size()) {
            center = _songs->at(index);
        }
    }
    
    //[self makeSectionSizes];
    [self.tableView reloadData];
    [self jumpToSong:center];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (shared_ptr<ISongArray>)songArray
{
    return _songArray;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    auto songsArray = [self songArray];
    if (!songsArray) {
        return 0;
    }
    _songs = songsArray->songs();
    
    if (scrollIndexNeeded && _songs->size() > 0) {
        ++fastScrollIndexCounter;
        static BOOL alreadyDisplayedIndex = NO;
        if (alreadyDisplayedIndex) {
            [fastScrollIndex setSongs:_songs];
        } else {
            auto retainedCounter = fastScrollIndexCounter;
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.75f * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                if (self->fastScrollIndexCounter == retainedCounter && scrollIndexNeeded) {
                    alreadyDisplayedIndex = YES;
                    [fastScrollIndex setSongs:_songs];
                }
            });
        }
    }
        
    auto size = _songs->size();
    //NSLog(@"size: %lu", size);
    return size;
}

- (SongEntry)songEntryAt:(NSIndexPath *)indexPath
{
#ifdef DEBUG
    //NSLog(@"song tableview: %@", NSStringFromCGRect(self.tableView.frame));
#endif
    return _songs->at([indexPath row]);
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    SongCell *cell = [self.tableView dequeueReusableCellWithIdentifier:@"NativeCell"];
    if (cell == nil) {
        cell = [[SongCell alloc] init];
    }
    
    SongEntry entry = [self songEntryAt:indexPath];
    auto theme = IApp::instance()->themeManager()->current();
    
//    cell.albumArt.image = shared_ptr<IPaintable>(new Color(0, 1, 1));
    
    // not needed anymore
    //cell.separator.image = theme->listSeparator();
    cell.separatorThickness = theme->listSeparatorThickness();
    cell.backgroundColor = [UIColor clearColor];
    [cell setSong:entry];

    return cell;
}

/*
- (void)drawRect:(CGRect)rect
{
    if (_background) {
        Gui::Painter::paint(_background, true);
    }
}
*/

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    auto songsArray = [self songArray];
    SongEntry entry = [self songEntryAt:indexPath];



	NSLog(@"ENTRY: %p", &entry);

    auto player = IApp::instance()->player();
    // we _mustn't_ check this, let the core handle it
    //if (entry != player->songEntryConnector().value()) {
        player->play(entry);
    //}

	[[SongsViewController sharedController] showPlayer];
}

- (BOOL)swipeBackPossible
{
    PlaylistCategory category = IApp::instance()->selectedPlaylistConnector().value().first;
    return (bool)category && !category.singularPlaylist();
}

- (IBAction)swipe:(id)sender
{
    if ([self swipeBackPossible]) {
        
        IApp::instance()->userSelectedPlaylist(shared_ptr<IPlaylist>());
    }
}

- (void)viewDidUnload {
    [self setSwipeGesture:nil];
    [super viewDidUnload];
}


- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
    if (gestureRecognizer == self.tapGesture) {
   
            return NO;
    } else {
        return YES;
    }
}

- (void)tap:(UIGestureRecognizer *)sender
{
//    UIView *keepSearchingView = self.searchView;
//    
//    CGPoint touchPoint = [sender locationInView:keepSearchingView];
//    if (!CGRectContainsPoint(keepSearchingView.bounds, touchPoint)) {
//        
//        [keepSearchingView resignFirstResponder];
//    }
}

// called by the player if nil
- (void)handleLongPress:(UILongPressGestureRecognizer *)gestureRecognizer
{
	SongEntry entry;
	if (gestureRecognizer) {

		if (gestureRecognizer.state != UIGestureRecognizerStateBegan){
			return;
		}
    
		CGPoint p = [gestureRecognizer locationInView:self.tableView];
    
		NSIndexPath *indexPath = [self.tableView indexPathForRowAtPoint:p];

		if (indexPath == nil) {
			return;
		}

		entry = [self songEntryAt:indexPath];
	} else {
		entry = IApp::instance()->player()->songEntryConnector().value();
	}
    
    [[SongActionMenu sharedMenu] actionForSong:entry songArray:[self songArray]];
}

@end

