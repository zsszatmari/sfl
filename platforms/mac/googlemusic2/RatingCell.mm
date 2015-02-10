//
//  RatingCell.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/14/12.
//
//

#include <map>
#import "RatingCell.h"
#import "AppDelegate.h"
#import "NSImage+Tinting.h"
#import "CocoaThemeManager.h"
#import "MainWindowController.h"
#import "SongsTableView.h"
#include "IApp.h"
#include "Painter.h"
#include "ThemeManager.h"
#include "ITheme.h"
#include "IPlayer.h"
#include "Tinter.h"
#include "NamedImage.h"
#import "CocoaThemeManager.h"
#include "IPreferences.h"

using namespace Gear;
using namespace Gui;

NSString * const kDefaultsKeyThumbsSwapped = @"SwapThumbsUpDown";

@interface RatingCell () {
    CGFloat thumbsWidth;
}
@end

@implementation RatingCell

static const int kThumbsUp = 1001;
static const int kThumbsDown = 1002;

//@synthesize song = _song;

static bool fiveStar()
{
    return IApp::instance()->preferences().boolForKey("FiveStarRatings");
}

- (int)numberOfIcons
{
    auto pSong = self.song;
    if (!pSong || pSong->uIntForKey("notInLibrary")) {
        return 0;
    }
    
    if (fiveStar()) {
        return 5;
    } else {
        return 1;
    }
}

static shared_ptr<IPaintable> ratingIcon(int rating, bool highlighted, int iconIndex)
{
    NSString *prefix = [[CocoaThemeManager sharedManager] themePrefix];
    if (fiveStar()) {
        if (prefix == nil) {
            if ((iconIndex+1) <= rating) {
                if (highlighted) {
                    return shared_ptr<IPaintable>(new NamedImage("ratingstar-white"));
                } else {
                    return shared_ptr<IPaintable>(new NamedImage("ratingstar-on"));
                }
            } else {
                if (highlighted) {
                    return shared_ptr<IPaintable>(new NamedImage("ratingstar-darkblue"));
                } else {
                    return shared_ptr<IPaintable>(new NamedImage("ratingstar"));
                }
            }
        } else {
            if ((iconIndex+1) <= rating) {
                return shared_ptr<IPaintable>(new NamedImage("ratingstar-on-moderndark"));
            } else {
                if (highlighted) {
                    return shared_ptr<IPaintable>(new NamedImage("ratingstar-darkblue"));
                } else {
                    return shared_ptr<IPaintable>(new NamedImage("ratingstar-moderndark"));
                }
            }
        }
    } else {
        Gui::Color selected;
        Gui::Color gray;
        if (prefix == nil) {
            // the same as the repeat/shuffle buttons selected color...
            // this is here temporarily, as proper theming is not yet finished
            
            if (highlighted) {
                selected = [[CocoaThemeManager sharedManager] songsRatingIconHighlightedColor];
                //selected = Gui::Color(124.0f/255.0f,126.0f/255.0f,129/255.0f);
            } else {
                //selected = Gui::Color(0.0f/255.0f,103.0f/255.0f,210.0f/255.0f);
                //selected = Gui::Color(124.0f/255.0f,126.0f/255.0f,129/255.0f);
                //selected = Gui::Color(81.0f/255.0f,79.0f/255.0f,80/255.0f);
                //selected = Gui::Color(115.0f/255.0f,130.0f/255.0f,142/255.0f);
                selected = Gui::Color(84.0/255.0f,84.0f/255.0f,84/255.0f);
                
            }
            if (highlighted) {
                gray = Gui::Color(1.0f, 1.0f, 1.0f, 1.0f);
            } else {
                gray = Gui::Color(204.0f/255.0f, 205.0f/255.0f, 206.0f/255.0f);
            }
        } else {
            // hackery, must be cleaned up at a later point
            selected = Gui::Color(207.0/255.0f, 205.0f/255.0f, 205.0f/255.0f);
            if (highlighted) {
                gray = Gui::Color(51.0/255.0f, 76.0f/255.0f, 150.0f/255.0f);
            } else {
                gray = Gui::Color(75.0/255.0f, 75.0f/255.0f, 75.0f/255.0f);
            }
        }
        
        if (rating == 5) {
            return Tinter::tintWithColor(shared_ptr<IPaintable>(new NamedImage("thumbsupfill")), selected);
        } else if (rating == 1) {
            return Tinter::tintWithColor(shared_ptr<IPaintable>(new NamedImage("thumbsdownfill")), selected);
            //return shared_ptr<IPaintable>(new NamedImage("thumbsdown-selected"));
        } else {
            return Tinter::tintWithColor(shared_ptr<IPaintable>(new NamedImage("thumbsupfill")), gray);
        }
        
        //return theme->ratingIcon(rating);
    }
}

- (NSImage *)imageAtIndex:(int)index respectFlipped:(BOOL *)respectFlipped yOffset:(CGFloat *)yOffset alpha:(float *)alpha
{
    auto song = self.song;
    if (!song) {
        return nil;
    }
    
    int rating;
    if (song) {
        rating = song->rating();
    } else {
        rating = 0;
    }
    *respectFlipped = YES;
    *alpha = 1.0f;
    *yOffset = 0;
    
    NSImage *ret = Painter::convertImage(*ratingIcon(rating, [self isHighlighted], index));
    // utter bullshit: must not do this:
    /*if (!fiveStar()) {
        [ret setSize:CGSizeMake(14, 15)];
    }*/
    return ret;
}

- (void)selectIndex:(int)index
{
    NSLog(@"select %d", index);
    auto song = self.song;
    if (!song) {
        return;
    }
    
    if (song) {
        if (fiveStar()) {
            IApp::instance()->player()->rate(song, index+1);
        } else {
            IApp::instance()->player()->rate(song);
        }
    }
    [[AppDelegate sharedDelegate].mainWindowController.musicListView reloadData];
}

- (int)thumbsFromIndex:(int)index
{
    BOOL swapped = [[NSUserDefaults standardUserDefaults] boolForKey:kDefaultsKeyThumbsSwapped];
    if ((index == 0 && !swapped) || (index == 1 && swapped)) {
        return kThumbsDown;
    } else {
        return kThumbsUp;
    }
}


@end
