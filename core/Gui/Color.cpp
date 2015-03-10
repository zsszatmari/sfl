//
//  Color.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "Color.h"
#include "IPainter.h"

namespace Gui
{
#define method Color::
    
    method Color(float red, float green, float blue, float alpha) :
        _red(red),
        _green(green),
        _blue(blue),
        _alpha(alpha)
    {
    }

    method Color(float white) :
        _red(white),
        _green(white),
        _blue(white),
        _alpha(1.0f)
    {
    }
    
    method Color() :
        _red(0),
        _green(0),
        _blue(0),
        _alpha(-1)
    {
    }
    
    method operator bool() const
    {
        return _alpha >= 0;
    }
    
    void method paint(const IPainter &painter) const
    {
        painter.paint(*this);
    }
    
    float method red() const
    {
        return _red;
    }
    
    float method green() const
    {
        return _green;
    }
    
    float method blue() const
    {
        return _blue;
    }
    
    float method alpha() const
    {
        return _alpha;
    }

    bool method operator==(const Color &rhs) const
    {  
        return _red == rhs._red && _green == rhs._green && _blue == rhs._blue && _alpha == rhs._alpha;
    }

    std::ostream &operator<<(std::ostream &stream, const Color &color)
    {
        stream << "r: " << color.red() << " g: " << color.green() << " b: " << color.blue() << " a: " << color.alpha();
        return stream;
    }
}
