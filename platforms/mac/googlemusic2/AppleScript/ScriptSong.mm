//
//  ScriptSong.m
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 02/04/14.
//
//

#import "ScriptSong.h"
#include "SongEntry.h"

using namespace Gear;

@implementation ScriptSong {
    Gear::SongEntry _entry;
}

- (id)initWithSong:(const SongEntry &)entry
{
    if (!entry) {
        return nil;
    }
    self = [super init];
    if (self) {
        _entry = entry;
    }
    return self;
}

- (Gear::SongEntry)entry
{
    return _entry;
}

@end
