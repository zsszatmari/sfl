//
//  Gradient.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/11/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__Gradient__
#define __G_Ear_core__Gradient__

#include <vector>
#include "IPaintable.h"

namespace Gui
{
    using std::vector;
    using std::pair;
    
    class Color;
    
    class core_export Gradient : public IPaintable
    {
    public:
        Gradient(const vector<pair<Color, float>> &colors);
        Gradient(const Color &from, const Color &to);
        const vector<pair<Color, float>> &colors() const;
        Color colorAt(float ratio) const;
        
        virtual void paint(const IPainter &painter) const;
        
    private:
        vector<pair<Color, float>> _colors;
    };
}

#endif /* defined(__G_Ear_core__Gradient__) */
