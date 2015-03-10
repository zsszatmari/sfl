//
//  LocalSessionImpl.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmari on 08/11/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <MediaPlayer/MediaPlayer.h>
#include "LocalSessionImpl.h"

namespace Gear
{
#define method LocalSessionImpl::
    
    void method refresh()
	{
        NSArray *items = [MPMediaQuery songsQuery].items;
        for (MPMediaItem *item in items) {
            NSSet *properties;
            //[item enumerateValuesForProperties:properties usingBlock: ];
        }
        
		//_favorites->addSongsLocally(songEntries);
    }
}