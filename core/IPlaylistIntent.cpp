//
//  IPlaylistIntent.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 06/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "IPlaylistIntent.h"

namespace Gear
{
#define method IPlaylistIntent::

    method ~IPlaylistIntent()
    {
    }
    
    bool method confirmationNeeded()
    {
        return !confirmationText().empty();
    }
}