//
//  IPaintable.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__IPaintable__
#define __G_Ear_core__IPaintable__

#include "Environment.h"

namespace Gui
{
    class IPainter;
    
    class core_export IPaintable
    {
    public:
        virtual void paint(const IPainter &painter) const = 0;
        virtual ~IPaintable();
    };
}

#endif /* defined(__G_Ear_core__IPaintable__) */
