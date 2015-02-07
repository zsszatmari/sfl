//
//  NamedImage.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "NamedImage.h"
#include "IPainter.h"

namespace Gui
{
#define method NamedImage::
    
    method NamedImage(const string &name, const Color &tintColor) :
        _imageName(name),
        _tintColor(tintColor)
    {
    }
    
    void method paint(const IPainter &painter) const
    {
        painter.paint(*this);
    }
    
    string method imageName() const
    {
        return _imageName;
    }
    
    const Color & method tintColor() const
    {
        return _tintColor;
    }
    
    void method setTintColor(const Color &tintColor)
    {
        _tintColor = tintColor;
    }
}
