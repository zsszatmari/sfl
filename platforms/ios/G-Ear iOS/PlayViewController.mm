//
//  PlayViewController.m
//  G-Ear iOS
//
//  Created by Gál László on 7/18/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "PlayViewController.h"
#import "SongsViewController.h"
#import "PlaylistListController.h"
#include "IApp.h"
#include "IPlayer.h"
#include "SignalConnection.h"
#include "StringUtility.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "Writer.h"
#include "Painter.h"
#include "Tinter.h"
#import "AppDelegate.h"
#import "RemoteApp.h"
#include "Debug.h"
#import "SongActionMenu.h"
#import "HelpOverlay.h"
#include "IPlaylist.h"
#include "ISession.h"
#import "EqViewController.h"
#include "IPreferences.h"
#import "NSDictionary+f.h"

using namespace Gear;
using namespace Gui;

@interface PlayViewController (){
    SignalConnection songEntryConnection;
    SignalConnection elapsedConnection;
    SignalConnection remainingConnection;
    SignalConnection ratioConnection;
    SignalConnection playingConnection;
    SignalConnection repeatConnection;
    SignalConnection shuffleConnection;
    SignalConnection ratingConnection;
	BOOL interrupted;
    SongEntry songEntry;
    SongIntents songIntents;
    UILabel *repeatOne;
    
    UIView *hideableContainer;
    NSArray *topIconConstraints;
    NSMutableArray *topIconRulers;
    NSArray *albumArtPortraitConstraints;
    NSTimer *hideTimer;
    NSLayoutConstraint *topDistance;
    NSLayoutConstraint *bottomDistance;
    
    UITapGestureRecognizer *viewTapRecognizer;

    UIButton *eqButton;
}

@end

@implementation PlayViewController

@synthesize artistLabel;
@synthesize songLabel;
@synthesize durationSlider;
@synthesize nextButton;
@synthesize shuffleButton;
@synthesize timeLeftLabel;
@synthesize currentTimeLabel;
@synthesize repeatButton;
@synthesize prevButton;
@synthesize playButton;
@synthesize shareButton;
@synthesize ratingButton;
@synthesize albumArt;

static const CGFloat sliderExtra = 16;

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
    }
    return self;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    UISwipeGestureRecognizer *swipeBack = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeBack:)];
    UISwipeGestureRecognizer *swipeForward = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(swipeForward:)];
    
    swipeBack.direction = UISwipeGestureRecognizerDirectionRight;
    swipeForward.direction = UISwipeGestureRecognizerDirectionLeft;
    
    [self.view addGestureRecognizer:swipeBack];
    [self.view addGestureRecognizer:swipeForward];
    
    [self.view setBackgroundColor:[UIColor blackColor]];
    
    hideableContainer = [[UIView alloc] init];
    hideableContainer.translatesAutoresizingMaskIntoConstraints = NO;
    viewTapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(viewTapped:)];
    [self.view addGestureRecognizer:viewTapRecognizer];
    [self.view addSubview:hideableContainer];
    
    
    UIView *topButtonRuler = [[UIView alloc] init];
    topButtonRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:topButtonRuler];
    UIView *songRuler = [[UIView alloc] init];
    songRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:songRuler];
    UIView *artistRuler = [[UIView alloc] init];
    artistRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:artistRuler];
    UIView *playRuler = [[UIView alloc] init];
    playRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:playRuler];
    UIView *songInfoRuler = [[UIView alloc] init];
    songInfoRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:songInfoRuler];
    UIView *songInfoInnerRuler = [[UIView alloc] init];
    songInfoInnerRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:songInfoInnerRuler];
    
    UIView *aboveTopButtonRuler = [[UIView alloc] init];
    aboveTopButtonRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:aboveTopButtonRuler];
    UIView *aboveSongInfoRuler = [[UIView alloc] init];
    aboveSongInfoRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:aboveSongInfoRuler];
    UIView *aboveArtRuler = [[UIView alloc] init];
    aboveArtRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:aboveArtRuler];
    UIView *belowArtRuler = [[UIView alloc] init];
    belowArtRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:belowArtRuler];
    UIView *belowPlayRuler = [[UIView alloc] init];
    belowPlayRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:belowPlayRuler];
    UIView *belowDurationRuler = [[UIView alloc] init];
    belowDurationRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:belowDurationRuler];
    UIView *durationRuler = [[UIView alloc] init];
    durationRuler.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:durationRuler];

    
    
    NSMutableDictionary *views =[NSDictionaryOfVariableBindings(hideableContainer,repeatButton,shuffleButton,songLabel,artistLabel,albumArt,playButton,prevButton,nextButton,durationSlider,topButtonRuler,songRuler,artistRuler,currentTimeLabel,timeLeftLabel,playRuler,songInfoRuler,aboveTopButtonRuler,aboveSongInfoRuler,aboveArtRuler,belowArtRuler,belowPlayRuler,belowDurationRuler,durationRuler
                                                                ) mutableCopy];
    
    for (int i = 0 ; i < 4 ; i++) {
        UIView *r = [[UIView alloc] init];
        r.translatesAutoresizingMaskIntoConstraints = NO;
        [hideableContainer addSubview:r];
        [views setObject:r forKey:[NSString stringWithFormat:@"r%d",i+1]];
    }
    
    [self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[hideableContainer]|" options:0 metrics:0 views:views]];
    [self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[hideableContainer]|" options:0 metrics:0 views:views]];

    artistLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:artistLabel];
    songLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:songLabel];
    albumArt.translatesAutoresizingMaskIntoConstraints = NO;
    
    [hideableContainer addSubview:ratingButton];
    ratingButton.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:shuffleButton];
    shuffleButton.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:repeatButton];
    repeatButton.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:shareButton];
    shareButton.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:durationSlider];
    durationSlider.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:timeLeftLabel];
    timeLeftLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:currentTimeLabel];
    currentTimeLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    [hideableContainer addSubview:playButton];
    playButton.translatesAutoresizingMaskIntoConstraints = NO;
    [playButton setTitle:nil forState:UIControlStateNormal];
    [hideableContainer addSubview:nextButton];
    nextButton.translatesAutoresizingMaskIntoConstraints = NO;
    [nextButton setTitle:nil forState:UIControlStateNormal];
    [hideableContainer addSubview:prevButton];
    prevButton.translatesAutoresizingMaskIntoConstraints = NO;
    [prevButton setTitle:nil forState:UIControlStateNormal];
    //playRuler.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.5f];
    //durationSlider.backgroundColor = [[UIColor greenColor] colorWithAlphaComponent:0.5f];

    eqButton = [UIButton buttonWithType:UIButtonTypeCustom];
    eqButton.translatesAutoresizingMaskIntoConstraints = NO;
    [repeatButton.superview addSubview:eqButton];
    [eqButton addTarget:self action:@selector(eqTapped:) forControlEvents:UIControlEventTouchUpInside];
    
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:topButtonRuler attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:shuffleButton attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    
    //CGFloat aboveSongMargin = 16;
    CGFloat betweenPlayAndSlider = 24 - sliderExtra;
    if ([[UIScreen mainScreen] bounds].size.height > 500) {
        // iphone 5 size
        //aboveSongMargin += 8;
        //betweenPlayAndSlider += 16 -sliderExtra;;
    }
    
    NSDictionary *metrics = @{//@"aboveSongMargin":@(aboveSongMargin),
                              @"betweenPlayAndSlider":@(betweenPlayAndSlider),
                              @"playHeight":@([UIImage imageNamed:@"play"].size.height),
                              @"sliderHeight":@(4+ 2*sliderExtra)};
    
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[aboveTopButtonRuler][topButtonRuler][aboveSongInfoRuler][songInfoRuler][aboveArtRuler]" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[belowArtRuler][playRuler][belowPlayRuler][durationRuler][belowDurationRuler]|" options:0 metrics:nil views:views]];
    
    /*
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[topButtonRuler]|" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[songInfoRuler]|" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[playRuler]|" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[durationRuler]|" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[aboveTopButtonRuler]|" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[aboveSongInfoRuler]|" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[aboveArtRuler]|" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[belowArtRuler]|" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[belowPlayRuler]|" options:0 metrics:nil views:views]];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[belowDurationRuler]|" options:0 metrics:nil views:views]];
    
    aboveTopButtonRuler.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.3f];
    aboveSongInfoRuler.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.3f];
    aboveArtRuler.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.3f];
    belowArtRuler.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.3f];
    belowPlayRuler.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.3f];
    belowDurationRuler.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.3f];
    
    topButtonRuler.backgroundColor = [[UIColor greenColor] colorWithAlphaComponent:0.3f];
    songInfoRuler.backgroundColor = [[UIColor greenColor] colorWithAlphaComponent:0.3f];
    playRuler.backgroundColor = [[UIColor greenColor] colorWithAlphaComponent:0.3f];
    durationRuler.backgroundColor = [[UIColor greenColor] colorWithAlphaComponent:0.3f];
    */
    //repeatButton.backgroundColor = [UIColor blueColor];
    
    
    albumArtPortraitConstraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[aboveArtRuler][albumArt][belowArtRuler]" options:0 metrics:metrics views:views];
    
    // these must not be equal to each other, but to their contents!
    
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:topButtonRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:0 constant:[UIImage imageNamed:@"repeat"].size.height]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:playRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:0 constant:[UIImage imageNamed:@"pause"].size.height]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:durationRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:0 constant:3]];
    
    
    /*[hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:topButtonRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:songInfoRuler attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:topButtonRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:playRuler attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:topButtonRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:durationSlider attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];*/
    
    // let the rulers be separate in the top and bottom region, with the album art vertically centered!
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:aboveTopButtonRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:aboveSongInfoRuler attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:aboveTopButtonRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:aboveArtRuler attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];
    
    // one of these. either let the rulers be separate in the top and bottom region, with the album art vertically centered, or vica versa
    //[self.view addConstraint:[NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:aboveArtRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:belowArtRuler attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];
    
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:belowArtRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:belowPlayRuler attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:belowArtRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:belowDurationRuler attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];
    
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:belowArtRuler attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:0 constant:23]];
    
         
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:albumArt attribute:NSLayoutAttributeWidth multiplier:1 constant:0]];
    //topDistance = [NSLayoutConstraint constraintWithItem:topButtonRuler attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeTop multiplier:1 constant:0];
    //[self.view addConstraint:topDistance];
    //bottomDistance = [NSLayoutConstraint constraintWithItem:durationSlider attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeBottom multiplier:1 constant:0];
    //[self.view addConstraint:bottomDistance];
    [self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[songRuler(==11)]-7-[artistRuler(==9)]" options:0 metrics:metrics views:views]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:songInfoInnerRuler attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:songInfoRuler attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:songInfoInnerRuler attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:songRuler attribute:NSLayoutAttributeTop multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:songInfoInnerRuler attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:artistRuler attribute:NSLayoutAttributeBottom multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:songInfoInnerRuler attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:songInfoRuler attribute:NSLayoutAttributeTop multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:songInfoInnerRuler attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:songInfoRuler attribute:NSLayoutAttributeBottom multiplier:1 constant:0]];

    
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:durationSlider attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:durationRuler attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:durationSlider attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil             attribute:NSLayoutAttributeNotAnAttribute multiplier:0 constant:30]];
    
    [self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-29-[songLabel]-29-|" options:0 metrics:nil views:views]];
    [self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-29-[artistLabel]-29-|" options:0 metrics:nil views:views]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:songLabel attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:songRuler attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:artistLabel attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:artistRuler attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    
    //prevButton.backgroundColor = [UIColor redColor];
    [hideableContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-13-[currentTimeLabel][r1][prevButton][r2][playButton][r3][nextButton][r4][timeLeftLabel]-13-|" options:NSLayoutFormatAlignAllCenterY metrics:nil views:views]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:playButton attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:playRuler attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:views[@"r2"] attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:views[@"r3"] attribute:NSLayoutAttributeWidth multiplier:1 constant:0]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:views[@"r1"] attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:views[@"r4"] attribute:NSLayoutAttributeWidth multiplier:1 constant:0]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:views[@"r1"] attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:views[@"r2"] attribute:NSLayoutAttributeWidth multiplier:1.25 constant:0]];

    
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:durationSlider attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:currentTimeLabel attribute:NSLayoutAttributeLeft multiplier:1 constant:+2]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:durationSlider attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:timeLeftLabel attribute:NSLayoutAttributeRight multiplier:1 constant:-2]];
    
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterX multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationLessThanOrEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationLessThanOrEqual toItem:self.view attribute:NSLayoutAttributeHeight multiplier:1 constant:0]];
    [self.view addConstraint:[NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationLessThanOrEqual toItem:self.view attribute:NSLayoutAttributeBottom multiplier:1 constant:0]];
    NSLayoutConstraint *beLarge = [NSLayoutConstraint constraintWithItem:albumArt attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1 constant:0];
    beLarge.priority = UILayoutPriorityDefaultHigh;
    [self.view addConstraint:beLarge];
    
    [self positionTopIcons];
    
    [self setOrientation:self.interfaceOrientation];
    
    //self.playButton.backgroundColor = [[UIColor greenColor] colorWithAlphaComponent:0.5f];
    //self.prevButton.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.5f];
    //self.nextButton.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.5f];
    
    if ([[UIScreen mainScreen] bounds].size.height > 500) {
        // iphone 5 size
        auto f = [](NSArray *views, int diff){
            for (UIView *view in views) {
                CGRect frame = view.frame;
                frame.origin.y += diff;
                view.frame = frame;
            }
        };
        f(@[self.artistLabel, self.songLabel], 8);
        f(@[self.playButton, self.prevButton, self.nextButton, self.currentTimeLabel, self.timeLeftLabel], -16);
    }
    repeatOne = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 20, 20)];
    [repeatOne setText:@"1"];
    [repeatOne setTextColor:Gui::Painter::convertColor(IApp::instance()->themeManager()->current()->selected())];
    repeatOne.center = CGPointMake(self.repeatButton.center.x + 17, self.repeatButton.center.y + 9);
    repeatOne.font = [UIFont systemFontOfSize:8.0f];
    repeatOne.translatesAutoresizingMaskIntoConstraints = NO;
    [hideableContainer addSubview:repeatOne];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:repeatOne attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:repeatButton attribute:NSLayoutAttributeCenterX multiplier:1 constant:17]];
    [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:repeatOne attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:repeatButton attribute:NSLayoutAttributeCenterY multiplier:1 constant:9]];
    
/*
	[[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
	[self becomeFirstResponder];
  */
	[RemoteApp set_remote: self];
    auto player = IApp::instance()->player();
    elapsedConnection = player->elapsedTimeConnector().connect([self](const string &str){
        self.currentTimeLabel.text = convertString(str);
    });
    
    remainingConnection = player->remainingTimeConnector().connect([self](const string &str){
        self.timeLeftLabel.text = convertString(str);
    });
    
    ratioConnection = player->songRatioConnector().connect([self](const float ratio){
        if (self.durationSlider.disableTime < [NSDate timeIntervalSinceReferenceDate]) {
            self.durationSlider.value = ratio;
        }
    });
    
    
    
    [self positionImage];
    
    songEntryConnection = player->songEntryConnector().connect([self](const SongEntry &aSongEntry) {
        
        if (songEntry == aSongEntry) {
            return;
        }
        
        songEntry = aSongEntry;
        
        auto song = songEntry.song();

        //BOOL ratePossible = NO;
        if (song) {
            self.artistLabel.text = convertString(song->artist());
            self.songLabel.text = convertString(song->title());
            //ratePossible = song->session()->possibleRatings().size() > 1;
        } else {
            self.artistLabel.text = @"";
            self.songLabel.text = @"";
        }
        
        [self positionTopIcons];
        
        // [[UIScreen mainScreen] scale] <= we need to address retina resolutions!
        self.albumArt.promise = IApp::instance()->player()->albumArt(self.albumArt.frame.size.width * [[UIScreen mainScreen] scale]);
   
        auto theme = IApp::instance()->themeManager()->current();
        BOOL shareActive = !songIntents.empty();
        
        self.shareButton.selected = shareActive;
        //[self.shareButton setImage:Painter::convertImage(theme->shareIcon(shareActive), self.shareButton.bounds.size) forState:UIControlStateNormal];
    });
    
    playingConnection = player->playingConnector().connect([self](const bool playing) {
        [self setPlaying:playing];
    });
    
    shuffleConnection = player->shuffle().connector().connect([self](const bool shuffle) {
        [self setShuffle:shuffle];
    });
    
    repeatConnection = player->repeat().connector().connect([self](const IPlayer::Repeat repeat) {
        [self setRepeat:repeat];
    });
    
    static NSMutableArray *ratingImages = nil;
    if (ratingImages == nil) {
        ratingImages = [NSMutableArray array];
        auto theme = IApp::instance()->themeManager()->current();
        for (int rating = 0 ; rating <= 5 ; ++rating) {
            [ratingImages addObject:Painter::convertImage(theme->ratingIcon(rating), CGSizeMake(40,40))];
        }
    }
    
    ratingConnection = player->rating().connector().connect([=](const int rating) {
        
        [self.ratingButton setImage:[ratingImages objectAtIndex:rating] forState:UIControlStateNormal];
    });
    
    [self applyTheme];
    
    auto theme = IApp::instance()->themeManager()->current();
    
    // we may not have the final size, this is especially problematic on ios 6
    [self.prevButton setImage:Painter::convertImage(theme->prevIcon(), CGSizeMake(40,40)/*self.prevButton.bounds.size*/) forState:UIControlStateNormal];
    [self.nextButton setImage:Painter::convertImage(theme->nextIcon(), CGSizeMake(40,40)) forState:UIControlStateNormal];
    
    void (^setImage)(UIButton *button, shared_ptr<IPaintable> icon, shared_ptr<IPaintable> iconInactive) = ^(UIButton *button, shared_ptr<IPaintable> iconActive, shared_ptr<IPaintable> iconInactive){
        
        [button setImage:Painter::convertImage(iconInactive, CGSizeMake(40,40)) forState:UIControlStateNormal];
        [button setImage:Painter::convertImage(iconActive, CGSizeMake(40,40)) forState:UIControlStateSelected];
    };
    setImage(self.shuffleButton, theme->shuffleIcon(true), theme->shuffleIcon(false));
    setImage(self.repeatButton, theme->repeatIcon(true), theme->repeatIcon(false));
    setImage(self.shareButton, theme->shareIcon(true), theme->shareIcon(false));
    setImage(eqButton, theme->eqIcon(true), theme->eqIcon(false));
    
    [self updateEqState];
    
    //[self.shareButton setImage:Painter::convertImage(theme->shareIcon(true), self.shareButton.bounds.size) forState:UIControlStateNormal];

#pragma mark interrupt handling

    if ([AppDelegate runningPreIOS6]) {
		[[AVAudioSession sharedInstance] setDelegate: self];
    } else {
		[[NSNotificationCenter defaultCenter] addObserver: self selector:
         @selector(on_interrupt:) name: AVAudioSessionInterruptionNotification object: nil];
        [[NSNotificationCenter defaultCenter] addObserver: self selector:
         @selector(routeChanged:) name: AVAudioSessionRouteChangeNotification object: nil];
        
    }
}

- (void)applyTheme
{
    auto theme = IApp::instance()->themeManager()->current();
    _bottomBar.image = theme->bottomBar();
    _topBar.image = theme->topBar();
    _backgroundView.image = theme->listBackground();
    
    Writer::apply(theme->miniplayerTitleText(), self.songLabel);
    Writer::apply(theme->miniplayerSubtitleText(), self.artistLabel);
    Writer::apply(theme->miniplayerTimeText(), self.timeLeftLabel);
    Writer::apply(theme->miniplayerTimeText(), self.currentTimeLabel);
    
    [durationSlider applyTheme];
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
                                duration:(NSTimeInterval)duration
{
    [super willRotateToInterfaceOrientation:toInterfaceOrientation duration:duration];
    
    [UIView animateWithDuration:duration animations:^{
        [self setOrientation:toInterfaceOrientation];
    }];
}
 
- (void)setOrientation:(UIInterfaceOrientation)orientation
{
    if (UIInterfaceOrientationIsPortrait(orientation)) {
        [self.view addConstraints:albumArtPortraitConstraints];
        [hideableContainer setBackgroundColor:[UIColor clearColor]];
        [self.backgroundView setAlpha:1.0f];
        [self.topBar setAlpha:1.0f];
        [self.bottomBar setAlpha:1.0f];
        
        [hideTimer invalidate];
        hideTimer = nil;
        
        [hideableContainer setAlpha:1.0f];
        topDistance.constant = 16;
        bottomDistance.constant = -30 + sliderExtra;

    } else {
        [self.view removeConstraints:albumArtPortraitConstraints];
        
        auto theme = IApp::instance()->themeManager()->current();
        [hideableContainer setBackgroundColor:Painter::convertColor(theme->overlayBackground())];
        [self.view setBackgroundColor:Painter::convertColor(theme->artBackground())];
        
        [self.backgroundView setAlpha:0.0f];
        [self.topBar setAlpha:0.0f];
        [self.bottomBar setAlpha:0.0f];
        
        topDistance.constant = 20;
        bottomDistance.constant = -18 + sliderExtra;
        
        [self setupHideTimer];
    }
    
    [self applyTheme];
    
    [self.view layoutIfNeeded];
}

- (void)setupHideTimer
{
    [hideTimer invalidate];
    hideTimer = [NSTimer scheduledTimerWithTimeInterval:10.0f target:self selector:@selector(hideContainer:) userInfo:nil repeats:NO];
}

- (void)hideContainer:(id)sender
{
    [UIView animateWithDuration:0.2f animations:^{
        [hideableContainer setAlpha:0.0f];
    }];
}

- (void)viewTapped:(id)sender
{
    if (!UIInterfaceOrientationIsPortrait(self.interfaceOrientation)) {
        if (hideableContainer.alpha < 0.01f) {
            [UIView animateWithDuration:0.2f animations:^{
                [hideableContainer setAlpha:1.0f];
            }];
        
            [self setupHideTimer];
        } else {
            CGPoint location = [viewTapRecognizer locationInView:self.view];
            CGPoint pauseOrigin = [self.view convertPoint:self.playButton.frame.origin fromView:self.playButton.superview];
            if (location.y < pauseOrigin.y) {
                [UIView animateWithDuration:0.2f animations:^{
                    [hideableContainer setAlpha:0.0f];
                }];
            }
        }
    }
}

- (void)positionTopIcons
{
    auto player = IApp::instance()->player();
    auto playlist = player->playlistCurrentlyPlaying();
    shared_ptr<ISongArray> songArray;
    if (playlist) {
        songArray = playlist->songArray();
    }
    
    // important side-effect:
    songIntents = [SongActionMenu songIntentsForSong:songEntry songArray:songArray];
    [self positionTopIconsRate:!songIntents.empty() action:!songIntents.empty()];
}

- (void)positionTopIconsRate:(BOOL)rate action:(BOOL)action
{
    if (!rate || !action) {
        // force only both or neither of them. this is needed because of help overlay
        rate = NO;
        action = NO;
    }
    
    for (NSLayoutConstraint *c in topIconConstraints) {
        [hideableContainer removeConstraint:c];
    }
    NSMutableArray *icons = [[NSMutableArray alloc] init];
    [icons addObject:repeatButton];
    [icons addObject:shuffleButton];
    
    [self.ratingButton setHidden:!rate];
    [self.ratingButton setEnabled:rate];
    
    [self.shareButton setHidden:!action];
    [self.shareButton setEnabled:action];

    if (topIconRulers == nil) {
        topIconRulers = [[NSMutableArray alloc] init];
        UIView *prev = nil;
        for (int i = 0 ; i < 6 ; i++) {
            UIView *r = [[UIView alloc] init];
            r.translatesAutoresizingMaskIntoConstraints = NO;
            [hideableContainer addSubview:r];
            [topIconRulers addObject:r];
            if (prev) {
                [hideableContainer addConstraint:[NSLayoutConstraint constraintWithItem:r attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:prev attribute:NSLayoutAttributeWidth multiplier:1 constant:0]];
            }
            prev = r;
        }
    }
    NSMutableDictionary *views = [NSDictionaryOfVariableBindings(repeatButton,shuffleButton,ratingButton,shareButton,eqButton) mutableCopy];
    [views addEntriesFromDictionary:@{@"r0":topIconRulers[0],
                                      @"r1":topIconRulers[1],
                                      @"r2":topIconRulers[2],
                                      @"r3":topIconRulers[3],
                                      @"r4":topIconRulers[4],
                                      @"r5":topIconRulers[5]}];
    
    int num = 2 + (rate ? 1 : 0) + (action ? 1 : 0) + 1;
    if (num == 5) {
        topIconConstraints = [NSLayoutConstraint constraintsWithVisualFormat:@"|[r0][repeatButton][r1][shuffleButton][r2][ratingButton][r3][eqButton][r4][shareButton][r5]|" options:NSLayoutFormatAlignAllCenterY metrics:nil views:views];
    } else {
        topIconConstraints = [NSLayoutConstraint constraintsWithVisualFormat:@"|[r0][repeatButton][r1][shuffleButton][r2][eqButton][r3]|" options:NSLayoutFormatAlignAllCenterY metrics:nil views:views];
    }
    [hideableContainer addConstraints:topIconConstraints];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationFade];
    
#ifdef PINCH_IS_HELP
    static UIPinchGestureRecognizer *pinch;
    pinch = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(pinchGesture:)];
    [self.view addGestureRecognizer:pinch];
#endif
    
    [self positionTopIcons];
    
    [self setOrientation:self.interfaceOrientation];
    
    [self updateEqState];
}

#ifdef PINCH_IS_HELP
- (void)pinchGesture:(UIGestureRecognizer *)sender
{
    if ([sender state] == UIGestureRecognizerStateEnded) {
        [HelpOverlay showForced:@"player"];
    }
    
}
#endif

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillAppear:animated];
    if (UIDeviceOrientationIsPortrait([[UIDevice currentDevice] orientation])) {
        [[UIApplication sharedApplication] setStatusBarHidden:NO withAnimation:UIStatusBarAnimationFade];
    }
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    NSDictionary *pieces = [@{@"eq": eqButton,
                             @"rate": ratingButton,
                             @"repeat": repeatButton,
                             @"shuffle": shuffleButton,
                             @"swipe": albumArt,
                             @"options": shareButton} filter:^BOOL(id key, UIView *view) {
                                 return !view.hidden;
                             }];
    [HelpOverlay showIfNecessary:@"player" fromController:self pieces:pieces];
    [HelpOverlay markIdentifier:@"player"];
}

// ios 5
- (void) beginInterruption {
	if (IApp::instance()->player()->playingConnector().value()) {
		IApp::instance()->player()->play();
		//[self setPlaying: NO];
		interrupted = YES;
	}
}

- (void) endInterruption {
	if (interrupted) {
		IApp::instance()->player()->play();
		//[self setPlaying: YES];
		interrupted = NO;
	}
}

// ios 6 ->
- (void) on_interrupt: (NSNotification *) notification
{
	NSUInteger type = [[notification.userInfo valueForKey: AVAudioSessionInterruptionTypeKey] unsignedIntegerValue];

	if (type == AVAudioSessionInterruptionTypeBegan && IApp::instance()->player()->playingConnector().value()) {
		IApp::instance()->player()->play();
		//[self setPlaying: NO];
		interrupted = YES;
	} else if (type == AVAudioSessionInterruptionTypeEnded && !IApp::instance()->player()->playingConnector().value() && interrupted) {
		IApp::instance()->player()->play();
		//[self setPlaying: YES];
		interrupted = NO;
	}
}

- (void)routeChanged:(NSNotification *) notification
{
    NSNumber *reason = [[notification userInfo] objectForKey:AVAudioSessionRouteChangeReasonKey];
    if ([reason integerValue] == AVAudioSessionRouteChangeReasonOldDeviceUnavailable && IApp::instance()->player()->playingConnector().value()) {
        IApp::instance()->player()->play();
    }
}

- (void)positionImage
{
    CGFloat topY = self.artistLabel.frame.origin.y + self.artistLabel.frame.size.height + 7.0f;
    CGFloat bottomY = self.playButton.frame.origin.y - 15.0f;
    
    CGFloat imageSize = bottomY - topY;
    if (imageSize > self.view.frame.size.width) {
        imageSize = self.view.frame.size.width;
    }
    
    CGRect rect = CGRectMake((self.view.frame.size.width - imageSize) /2,
                             topY + (bottomY - topY - imageSize) /2,
                             imageSize,
                             imageSize);
    self.albumArt.frame = rect;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)ratioChanged:(id)sender
{
    self.durationSlider.disableTime = [NSDate timeIntervalSinceReferenceDate] + 1.5f;
    IApp::instance()->player()->setRatio(self.durationSlider.value);
}

- (IBAction)playTapped:(id)sender
{
    // immediately invert (otherwise slow on ios 6)
    [self setPlaying:((bool)IApp::instance()->player()->songEntryConnector().value()) && !IApp::instance()->player()->playingConnector().value()];
    
    IApp::instance()->player()->play();
}

- (IBAction)prevTapped:(id)sender
{
    IApp::instance()->player()->prev();
}

- (IBAction)nextTapped:(id)sender
{
    IApp::instance()->player()->next();
}

- (void)viewDidUnload
{
	[self setDurationSlider:nil];
    [self setTimeLeftLabel:nil];
    [self setCurrentTimeLabel:nil];
    [self setAlbumArt:nil];
    [self setArtistLabel:nil];
    [self setSongLabel:nil];
    [super viewDidUnload];
}

- (void)swipeBack:(UISwipeGestureRecognizer *)sender
{
     CGPoint coords = [sender locationInView:sender.view];
    if (coords.y > self.playButton.frame.origin.y + self.playButton.frame.size.height) {
         return;
    }
	((SongsViewController *) self.navigationController.delegate).transition_dir = TRANSITION_FROM_LEFT;
	[self.navigationController popViewControllerAnimated: YES];
}

- (void)swipeForward:(UISwipeGestureRecognizer *)sender
{
    [self nextTapped:nil];
}

- (void)setPlaying:(BOOL)playing
{
    auto theme = IApp::instance()->themeManager()->current();
    
    [self.playButton setImage:Painter::convertImage(playing ? theme->pauseIcon() : theme->playIcon(),CGSizeMake(40,40)) forState:UIControlStateNormal];
    //self.playButton.titleLabel.text = playing ? @"Stop" : @"Play";
}

- (void)eqTapped:(id)sender
{
    [EqViewController showFrom:self];
}

- (void)updateEqState
{
    [eqButton setSelected:IApp::instance()->preferences().boolForKey("eqEnabled")];
}

- (IBAction)repeatTapped:(UIButton *)sender
{
    // selected state is inverse, but we still have the old state
    if ([[NSUserDefaults standardUserDefaults] boolForKey:@"SingleSongLoopPossible"]) {
        auto value = IApp::instance()->player()->repeat().get();
        switch(value) {
            case Gear::IPlayer::Repeat::Off:
                value = Gear::IPlayer::Repeat::On;
                break;
            case Gear::IPlayer::Repeat::On:
                value = Gear::IPlayer::Repeat::OneSong;
                break;
            case Gear::IPlayer::Repeat::OneSong:
                value = Gear::IPlayer::Repeat::Off;
                break;
        }
        IApp::instance()->player()->repeat() = value;
    } else {
        IApp::instance()->player()->repeat() = IPlayer::Repeat(!sender.selected);
    }
}

- (IBAction)shuffleTapped:(UIButton *)sender
{
    auto player = IApp::instance()->player();
    auto &shuffle = player->shuffle();
    BOOL selected = !sender.selected;
    //bool prevValue = shuffle;
    //prevValue = prevValue;
    shuffle = selected;
}

- (void)setRepeat:(IPlayer::Repeat)repeat
{
    self.repeatButton.selected = repeat != IPlayer::Repeat::Off;
    repeatOne.hidden = repeat != IPlayer::Repeat::OneSong;
}

- (void)setShuffle:(BOOL)shuffle
{
    self.shuffleButton.selected = shuffle;
}

- (IBAction)shareTapped:(id)sender
{
    auto player = IApp::instance()->player();
    auto song = player->songEntryConnector().value();
    auto playlist = player->playlistCurrentlyPlaying();
    if (song && playlist) {
        [[SongActionMenu sharedMenu] actionForSong:song songIntents:songIntents];
    }
}

- (IBAction)ratingTapped:(id)sender
{
    IApp::instance()->player()->rate();
}

@end
