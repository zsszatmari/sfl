//
//  ScriptSong.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 02/04/14.
//
//

#import <Foundation/Foundation.h>
#include "SongEntry.h"

@interface ScriptSong : NSObject

- (id)initWithSong:(const Gear::SongEntry &)entry;
- (Gear::SongEntry)entry;

@end
