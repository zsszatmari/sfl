//
//  GearTopBar.m
//  G-Ear iOS
//
//  Created by Gál László on 9/23/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "AppDelegate.h"
#import "GearTopBar.h"
#import "SongsViewController.h"
#include "IApp.h"
#include "IPlayer.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "Painter.h"
#include "Writer.h"
#include "SongPredicate.h"
#include "StringUtility.h"
#include "TextAttributes.h"
#import "NSObject+Ext.h"
#import "UIView+Ext.h"
#include "IPlaylist.h"
#include "ISongArray.h"
#include "Chain.h"
#include "ValidPtr.h"
#include "Debug.h"

using namespace Gear;
using namespace Gui;

@implementation GearTopBar {
    UILabel *nowPlayingLabel;
    UILabel *searchLabel;
    SignalConnection playingConnection;
    SignalConnection playlistConnection;
    SignalConnection playlistCategoryConnection;
    UISearchBar *topSearchBar;
    UIControl *gestureCacher;
    BOOL shouldBeginEditing;
    BOOL showSearchBar;
    weak_ptr<IPlaylist> lastPlaylist;
    NSLayoutConstraint *searchBarHorizontalConstraint;
    NSLayoutConstraint *topGapConstraint;
}


static CGFloat kStatusBarHeight = 22.0f;
static bool minimal_searchbar = NO;

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) {

        searchLabel = [[UILabel alloc] init];
        searchLabel.text = @"Search";
        searchLabel.backgroundColor = [UIColor clearColor];
        //searchLabel.backgroundColor = [UIColor redColor];
        searchLabel.textAlignment = NSTextAlignmentLeft;

        nowPlayingLabel = [[UILabel alloc] init];
        nowPlayingLabel.text = @"Now playing";
        nowPlayingLabel.backgroundColor = [UIColor clearColor];
        nowPlayingLabel.textAlignment = NSTextAlignmentRight;

#ifdef SPLASH_SCREEN
        searchLabel.hidden = YES;
        nowPlayingLabel.hidden = YES;
        return self;
#endif
     
        topSearchBar = [[UISearchBar alloc ] init];
        topSearchBar.delegate = self;

        if ([AppDelegate runningPreIOS7]) {
            topSearchBar.backgroundImage = [[UIImage alloc] init];
        } else {
            if (minimal_searchbar) {
                topSearchBar.searchBarStyle = UISearchBarStyleMinimal;
            } else {
                topSearchBar.backgroundImage = [[UIImage alloc] init];
            }
            
            ((UITextField *) [[topSearchBar.recursive_subviews filteredArrayUsingPredicate:
                [NSPredicate predicateWithFormat: @"class.description like 'UISearchBarTextField'"]]
                firstObject]).textColor = [UIColor grayColor];
        }
        
        searchLabel.userInteractionEnabled = YES;
        nowPlayingLabel.userInteractionEnabled = YES;
        
        topSearchBar.translatesAutoresizingMaskIntoConstraints = NO;
        searchLabel.translatesAutoresizingMaskIntoConstraints = NO;
        nowPlayingLabel.translatesAutoresizingMaskIntoConstraints = NO;
        
        [self addSubview:topSearchBar];
        [self addSubview:searchLabel];
        [self addSubview:nowPlayingLabel];
        UIView *verticalRuler = [[UIView alloc] init];
        verticalRuler.translatesAutoresizingMaskIntoConstraints = NO;
        verticalRuler.hidden = YES;
        [self addSubview:verticalRuler];
        UIView *horizontalRuler = [[UIView alloc] init];
        horizontalRuler.translatesAutoresizingMaskIntoConstraints = NO;
        horizontalRuler.hidden = YES;
        [self addSubview:horizontalRuler];
        
        CGSize searchLabelSize = [searchLabel.text sizeWithFont:searchLabel.font];
        CGSize nowPlayingLabelSize = [nowPlayingLabel.text sizeWithFont:nowPlayingLabel.font];
        //searchLabelSize.height *= 2*1.5;
        searchLabelSize.width = nowPlayingLabelSize.width;
        //nowPlayingLabelSize.height *= 2*1.5;
        //nowPlayingLabel.backgroundColor = [UIColor redColor];
        //verticalRuler.backgroundColor = [UIColor greenColor];
        
        NSDictionary *views = NSDictionaryOfVariableBindings(topSearchBar,searchLabel,nowPlayingLabel,verticalRuler);
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-5-[searchLabel(==searchLabelWidth)]-[verticalRuler]-[nowPlayingLabel(==nowPlayingWidth)]-5-|" options:NSLayoutFormatAlignAllCenterY metrics:@{@"searchLabelWidth":@(searchLabelSize.width),@"nowPlayingWidth":@(nowPlayingLabelSize.width)} views:views]];
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[verticalRuler]|" options:0 metrics:nil views:views]];
        topGapConstraint = [NSLayoutConstraint constraintWithItem:verticalRuler attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:verticalRuler.superview attribute:NSLayoutAttributeTop multiplier:1 constant:0];
        [self addConstraint:topGapConstraint];
        [self setTopGap];
        
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[searchLabel(==searchLabelHeight)]" options:0 metrics:@{@"searchLabelHeight":@(searchLabelSize.height)} views:views]];
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[nowPlayingLabel(==nowPlayingLabelHeight)]" options:0 metrics:@{@"nowPlayingLabelHeight":@(nowPlayingLabelSize.height)} views:views]];
        [self addConstraint:[NSLayoutConstraint constraintWithItem:horizontalRuler attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:searchLabel attribute:NSLayoutAttributeLeft multiplier:1 constant:0]];
        [self addConstraint:[NSLayoutConstraint constraintWithItem:horizontalRuler attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:nowPlayingLabel attribute:NSLayoutAttributeLeft multiplier:1 constant:0]];
        
        [self addConstraint:[NSLayoutConstraint constraintWithItem:topSearchBar attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:horizontalRuler attribute:NSLayoutAttributeWidth multiplier:1 constant:0]];
        [self addConstraint:[NSLayoutConstraint constraintWithItem:topSearchBar attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:verticalRuler attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
        [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[topSearchBar]|" options:0 metrics:nil views:views]];
        searchBarHorizontalConstraint = [NSLayoutConstraint constraintWithItem:topSearchBar attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:horizontalRuler attribute:NSLayoutAttributeLeft multiplier:1 constant:0];
        [self addConstraint:searchBarHorizontalConstraint];
        
        [self applyTheme];
        
        playingConnection = IApp::instance()->player()->playingConnector().connect([self](const bool playing) {
    ////               nowPlayingLabel.hidden = !playing;
            auto theme = IApp::instance()->themeManager()->current();
            Gui::Writer::apply(playing ? theme->navigationTextActive() : theme->navigationText(), nowPlayingLabel);

        });
        dispatch_async(dispatch_get_main_queue(), ^{
            playlistCategoryConnection = IApp::instance()->selectedPlaylistConnector().connect([self](const pair<PlaylistCategory,shared_ptr<IPlaylist>> &p){
                
                if (p.second) {
                    // this is wrong, we mustn't do this here!
                    /*
                    shared_ptr<IPlaylist> singularPlaylist = rhs.playlists()[0];
                    IApp::instance()->userSelectedPlaylist(singularPlaylist);*/

                    if (lastPlaylist.lock() != p.second) {
                        lastPlaylist = p.second;
                        [self songListShown];
                    }
                } else {
                    lastPlaylist.reset();
                    [self songListHidden];
                }
                
            });
        });
        
        gestureCacher = [[UIControl alloc] init];
        gestureCacher.translatesAutoresizingMaskIntoConstraints = NO;
        
        UITapGestureRecognizer* tapSearchLabel = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapLabel:)];
        
        UITapGestureRecognizer* tapNowPlaying = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapLabel:)];
        
        UITapGestureRecognizer* tapCacher = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapOutside:)];
        
        
        [searchLabel setUserInteractionEnabled:YES];
        [searchLabel addGestureRecognizer:tapSearchLabel];
        
        [nowPlayingLabel setUserInteractionEnabled:YES];
        [nowPlayingLabel addGestureRecognizer:tapNowPlaying];
        
        [gestureCacher setUserInteractionEnabled:YES];
        [gestureCacher addGestureRecognizer:tapCacher];
        // this is necessary because sometimes the hidden view just remains there making everything (except tap) impossible, but people do not tap in this case
        [gestureCacher addTarget:self action:@selector(dismissKeyboard:) forControlEvents:UIControlEventTouchUpInside];
        
        //gestureCacher.backgroundColor = [[UIColor redColor] colorWithAlphaComponent:0.4f];
        gestureCacher.backgroundColor = [UIColor clearColor];
        
        topSearchBar.hidden = YES;
        searchLabel.hidden = NO;
////        nowPlayingLabel.hidden = YES;
        shouldBeginEditing = YES;
    }
    return self;
}

- (void)applyTheme
{
    auto theme = IApp::instance()->themeManager()->current();
    
    Writer::apply(theme->navigationText(), searchLabel);
    Writer::apply(theme->navigationText(), nowPlayingLabel);
    
    self.image = theme->topBar();
}

- (void)willRotate
{
    [self setTopGap];
}

- (void)setTopGap
{
    CGFloat gapY = [[self class] statusBarHeight];
    topGapConstraint.constant = gapY;
}

- (void)songListShown
{
    SongsViewController *songsView = [SongsViewController sharedController];
    BOOL onSettings = songsView.settingsActive || songsView.tabBar.selected_tag == SETTINGS_TAG;
    if (onSettings) {
        return;
    }
    
    string value;
    auto selected = IApp::instance()->selectedPlaylist();
    bool shouldShow = false;
    
    
    if (selected) {
        value = selected->filterPredicate().value();
        // show search bar when going to search playlist and there are no results displayed;
        shouldShow = !value.empty();
        if (!shouldShow) {
            shouldShow = (!onSettings) && selected->songArray()->needsPredicate() && selected->songArray()->songs()->size() == 0;
            if (shouldShow) {
                [topSearchBar becomeFirstResponder];
            }
        }
    }
    if (!shouldShow) {
        [topSearchBar setText:@""];
        [self setFilter:""];
        [self hideSearchBar];
            
        [self showSearchLabel];
    } else {
        [topSearchBar setText:convertString(value)];
        [self setFilter:value];
        [self showSearchBar];
        
        [self hideSearchLabel];
    }
}

- (void)songListHidden
{
    // if he/she was searching in a playlist, then went back to list of playlists, we should hide search field
    [topSearchBar setText:@""];
    [self setFilter:""];
    [self hideSearchBarAndShowLabel:NO];
 
    [self hideSearchLabel];
}

- (void)tapLabel:(UIGestureRecognizer*)recognizer
{
    if( [recognizer state] == UIGestureRecognizerStateEnded ) {
        UILabel *label = (UILabel*)[recognizer view];
        if (label == nowPlayingLabel) {
            [[SongsViewController sharedController] showPlayer];
        }
        
        if (label == searchLabel) {
            
            [topSearchBar becomeFirstResponder];
            [self showSearchBar];
        }
    }
}

- (void)showSearchLabel
{
#ifdef SPLASH_SCREEN
    return;
#endif
    [UIView animateWithDuration:0.3f animations:^{
        [searchLabel setAlpha:1.0f];
    }];
}

- (void) hideSearchLabel
{
#ifdef SPLASH_SCREEN
    return;
#endif
	[UIView animateWithDuration: .3 animations: ^{
		[searchLabel setAlpha: .0];
	}];
}

- (void)settingsShown
{
    lastPlaylist.reset();
    [self hideSearchLabel];
}

- (void)settingsHidden
{
    [self songListShown];
}

- (void)showKeyboard
{
    NSUInteger idx = [self.superview.subviews indexOfObject:gestureCacher];
    if (idx == NSNotFound) {
        [topSearchBar becomeFirstResponder];
        [self addGestureCacher];
    }
}

- (void)hideKeyboard
{
    NSUInteger idx = [self.superview.subviews indexOfObject:gestureCacher];
    if (idx != NSNotFound) {
        [topSearchBar resignFirstResponder];
        [gestureCacher removeFromSuperview];
    }
}

- (void)dismissKeyboard:(id)sender
{
    [self hideKeyboard];
    if (topSearchBar.text.length == 0) {
        [self hideSearchBar];
    }
}

- (void)tapOutside:(UIGestureRecognizer*)recognizer
{
    if( [recognizer state] == UIGestureRecognizerStateEnded ) {
        if ([recognizer view] == gestureCacher) {
            [self dismissKeyboard:recognizer];
        }
    }
}

- (void)setFilter:(const string &)filter
{
    auto playlist = IApp::instance()->selectedPlaylist();
    if (playlist) {
        playlist->setFilterPredicate(SongPredicate("", filter, SongPredicate::Contains()));
    }
}

- (void)searchBar:(UISearchBar *)searchBar textDidChange:(NSString *)searchText
{
    [self setFilter:convertString(searchText)];
}

- (void)searchBarSearchButtonClicked:(UISearchBar *)searchBar
{
    [self setFilter:convertString([searchBar text])];
}

- (BOOL)searchBarShouldBeginEditing:(UISearchBar *)bar {
    BOOL result = shouldBeginEditing;
    shouldBeginEditing = YES;
    return result;
}

- (void)searchBarTextDidBeginEditing:(UISearchBar*)searchBar
{
    [self showKeyboard];
}

- (void)addGestureCacher
{
    [self.superview addSubview:gestureCacher];
    NSDictionary *views = NSDictionaryOfVariableBindings(gestureCacher,topSearchBar);
    [self.superview addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[gestureCacher]|" options:0 metrics:nil views:views]];
    [self.superview addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[topSearchBar][gestureCacher]-40-|" options:0 metrics:nil views:views]];
}

- (void)showSearchBar
{
#ifdef SPLASH_SCREEN
    return;
#endif
    
    if (showSearchBar) {
        return;
    }
    showSearchBar = YES;
    [self hideSearchLabel];
    
    [self addGestureCacher];
    topSearchBar.hidden = NO;
    topSearchBar.alpha = 0;
	if ([AppDelegate runningPreIOS7]) {
        [UIView animateWithDuration:0.5
                              delay:0.0
                            options: UIViewAnimationCurveEaseOut
                         animations:^{
                             
            topSearchBar.alpha = 1;
            searchLabel.alpha = 0;
        } completion:^(BOOL finished){
        }];
	} else {
//		topSearchBar.alpha = 1.;
		searchLabel.alpha = 0.;
        
        searchBarHorizontalConstraint.constant = -260;
        [self layoutIfNeeded];
        [UIView animateWithDuration: .7 delay: .0 usingSpringWithDamping: .53 initialSpringVelocity: 1. options: UIViewAnimationOptionTransitionNone
		animations: ^{
            searchBarHorizontalConstraint.constant = 0;
			topSearchBar.alpha = 1.;
            
            [self layoutIfNeeded];
		}
		completion: ^(BOOL finished) {
		}];
	}
}

- (void)hideSearchBar
{
    [self hideSearchBarAndShowLabel:YES];
}

- (void)hideSearchBarAndShowLabel:(BOOL)shouldShowLabel
{
    if (!showSearchBar) {
        return;
    }
    showSearchBar = NO;
    
    [self hideKeyboard];
	if ([AppDelegate runningPreIOS7]) {
        topSearchBar.alpha = 1;
        [UIView animateWithDuration:0.5
                          delay:0.0
                        options: UIViewAnimationCurveEaseOut
                     animations:^{
                         topSearchBar.alpha = 0;
                         searchLabel.alpha = 1;
                     }
                     completion:^(BOOL finished){
                         topSearchBar.hidden = YES;
                     }];
	} else {
        [self layoutIfNeeded];
		[UIView animateWithDuration: .15 delay: .0 options: UIViewAnimationCurveEaseOut
		animations: ^{
			
            searchBarHorizontalConstraint.constant = -260;
            [self layoutIfNeeded];
		}
		completion: ^(BOOL finished) {
			topSearchBar.alpha = .0;
			topSearchBar.hidden = YES;
            
            if (shouldShowLabel) {
                [UIView animateWithDuration: .15 delay: .0 options: UIViewAnimationCurveEaseOut
                animations: ^{
                    searchLabel.alpha = 1.;
                }
                completion: ^(BOOL finished) {
                }];
            }
		}];

	}
}

+ (CGFloat)statusBarHeight
{
    BOOL portrait = UIInterfaceOrientationIsPortrait([UIApplication sharedApplication].statusBarOrientation);
    return ([AppDelegate runningPreIOS7] || !portrait) ? 0 : kStatusBarHeight;
}

@end
