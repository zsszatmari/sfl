//
//  IPlaylistIntent.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 06/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef G_Ear_core_IPlaylistIntent_h
#define G_Ear_core_IPlaylistIntent_h

#include <string>

namespace Gear
{
    class IPlaylistIntent
    {
    public:
        virtual ~IPlaylistIntent();
        virtual const std::string menuText() const = 0;
        virtual bool confirmationNeeded();
        virtual const std::string confirmationText() const = 0;
        virtual void apply() = 0;
    };
}

#endif
