//
//  StatusBarManager.h
//  G-Ear
//
//  Created by Zsolt Szatmari on 6/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

extern NSString *kUserDefaultsStatusBarKey;

@protocol ControlManagerDelegate;

@interface StatusBarManager : NSObject

@property(unsafe_unretained, nonatomic) id<ControlManagerDelegate> delegate;

- (void)setupStatusBar;
- (void)setPlaying:(BOOL)playing;
- (void)setDisplayedAlbumArtist:(NSString *)albumArtist song:(NSString *)song artist:(NSString *)aArtist;

@end

@protocol ControlManagerDelegate 

- (void)togglePlay:(id)sender;
- (void)prev:(id)sender;
- (void)next:(id)sender;
- (void)statusTextClicked:(id)sender;

@end