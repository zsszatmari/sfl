//
//  ScriptPlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 30/03/14.
//
//

#import <Foundation/Foundation.h>
#include "stdplus.h"
#include "IPlaylist.h"

@class ScriptSong;

@interface ScriptPlaylist : NSObject

- (id)initWithPlaylist:(shared_ptr<Gear::IPlaylist>)playlist;
- (void)addSong:(ScriptSong *)song;

@end
