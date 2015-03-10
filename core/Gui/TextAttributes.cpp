//
//  TextAttributes.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "TextAttributes.h"

namespace Gui
{
#define method TextAttributes::
    
    method TextAttributes() :
        _font(Font::Category)
    {
    }
    
    method TextAttributes(const Font font) :
        _font(font)
    {
    }
    
    method TextAttributes(const Font font, const Color &color) :
        _font(font),
        _color(color)
    {
    }
    
    const TextAttributes::Font method font() const
    {
        return _font;
    }
    
    const Color & method color() const
    {
        return _color;
    }
    
    void method setColor(const Color &color)
    {
        _color = color;
    }
}
