//
//  IPainter.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__IPainter__
#define __G_Ear_core__IPainter__

#include "Environment.h"

namespace Gui
{
    class Color;
    class NamedImage;
    class BitmapImage;
    class Gradient;
    
    class core_export IPainter
    {
    public:
        virtual void paint(const Color &color) const = 0;
        virtual void paint(const NamedImage &image) const = 0;
        virtual void paint(const BitmapImage &image) const = 0;
        virtual void paint(const Gradient &gradient) const = 0;
        virtual ~IPainter();
    };
}

#endif /* defined(__G_Ear_core__IPainter__) */
