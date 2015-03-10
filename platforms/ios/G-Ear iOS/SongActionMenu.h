//
//  SongActionMenu.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 24/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <vector>
#include "stdplus.h"

namespace Gear
{
    class SongEntry;
    class ISongArray;
    class ISongIntent;
}

struct SongIntents
{
    std::vector<shared_ptr<Gear::ISongIntent>> intents;
    std::vector<std::pair<std::string, std::vector<shared_ptr<Gear::ISongIntent>>>> submenus;
    
    bool empty() const
    {
        return intents.empty() && submenus.empty();
    }
};

@interface SongActionMenu : NSObject<UIActionSheetDelegate,UIAlertViewDelegate>

+ (SongActionMenu *)sharedMenu;
+ (SongIntents)songIntentsForSong:(const Gear::SongEntry &)entry songArray:(const shared_ptr<Gear::ISongArray> &)songsArray;
- (void)actionForSong:(const Gear::SongEntry &)entry songArray:(const shared_ptr<Gear::ISongArray> &)songsArray;
- (void)actionForSong:(const Gear::SongEntry &)entry songIntents:(const SongIntents &)songIntents;

@end
