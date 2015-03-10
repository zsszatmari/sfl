//
//  ShortcutsController.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 1/30/13.
//
//

#import <Foundation/Foundation.h>

extern NSString * const kKeyShortcutThumbsUp;
extern NSString * const kKeyShortcutThumbsDown;
extern NSString * const kKeyShortcutPlayPause;
extern NSString * const kKeyShortcutNext;
extern NSString * const kKeyShortcutPrevious;
extern NSString * const kKeyShortcutJumpToCurrent;

@class MASShortcutView;

@interface ShortcutsController : NSObject
@property (unsafe_unretained) IBOutlet MASShortcutView *thumbsUpView;
@property (unsafe_unretained) IBOutlet MASShortcutView *thumbsDownView;
@property (unsafe_unretained) IBOutlet MASShortcutView *playPauseView;
@property (unsafe_unretained) IBOutlet MASShortcutView *nextView;
@property (unsafe_unretained) IBOutlet MASShortcutView *previousView;
@property (unsafe_unretained) IBOutlet MASShortcutView *jumpToCurrentView;
@property (unsafe_unretained) IBOutlet MASShortcutView *volumeUpView;
@property (unsafe_unretained) IBOutlet MASShortcutView *volumeDownView;

@end
