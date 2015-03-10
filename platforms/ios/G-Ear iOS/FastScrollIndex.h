//
//  FastScrollIndex.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 10/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <map>
#include "ValidPtr.h"
#include "Chain.h"

namespace Gear
{
    class SongEntry;
    class IPlaylist;
}

static const int kFastScrollSlots = 'Z' - 'A' + 1;

@interface FastScrollIndex : UIView

extern NSString * const kNotificationFastScroll;

//- (void)setSongNames:(const std::vector<std::string> &)names;
- (void)setMapping:(const std::map<int,std::string> &)mapping;
//- (void)setSongs:(Gear::ValidPtr<const Gear::Chain<Gear::SongEntry>> &)songs;
- (void)setPlaylists:(const std::vector<shared_ptr<Gear::IPlaylist>> &)playlists;

@end
