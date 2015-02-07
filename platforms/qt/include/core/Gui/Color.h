//
//  Color.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__Color__
#define __G_Ear_core__Color__

#include "IPaintable.h"
#include "Environment.h"

namespace Gui
{
    class IPainter;
    
    class core_export Color final : public IPaintable
    {
    public:
        Color();
        Color(float red, float green, float blue, float alpha = 1.0f);
        Color(float white);
        
        float red() const;
        float green() const;
        float blue() const;
        float alpha() const;
        
        operator bool() const;
        
        virtual void paint(const IPainter &painter) const;
        
    private:
        float _red;
        float _green;
        float _blue;
        float _alpha;
    };
}

#endif /* defined(__G_Ear_core__Color__) */
