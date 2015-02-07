//
//  PlaylistManager.h
//  G-Ear
//
//  Created by Zsolt Szatmari on 7/12/12.
//
//

#import <Foundation/Foundation.h>
#include <memory>
#import "CategoryField.h"

namespace Gear
{
    class IPlaylist;
}

@class Song;
@class Playlist;
@protocol PlaylistProtocol;

static const CGFloat kCategoryPositionLeft = 6.0f;
static const CGFloat kCategoryPositionTopbottom = kCategoryPositionLeft;
static const CGFloat kPlaylistPositionLeftIfIconPresent = 18.0f + 14.0f + 6.0f;
static const CGFloat kPlaylistIconPositionLeft = 18.0f;
static const CGFloat kPlaylistPositionLeft = 18.0f;

@interface PlaylistManager : NSObject<NSTextFieldDelegate, cellActionDelegate>
@property (unsafe_unretained) IBOutlet NSScrollView *scrollView;

+ (BOOL)isSelectable:(id)item;
- (void)startEditing:(std::shared_ptr<Gear::IPlaylist>)playlist;
- (void)setupCategories;
- (void)applyTheme;

@end
