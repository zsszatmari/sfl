//
//  Tinter.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "Tinter.h"
#include "IPaintable.h"
#include "NamedImage.h"

namespace Gui
{
#define method Tinter::
    
    shared_ptr<IPaintable> method tintWithColor(const shared_ptr<IPaintable> &icon, const Color &color)
    {
        Tinter tinter(color);
        if (icon) {
            icon->paint(tinter);
        }
        
        if (tinter._result) {
            return tinter._result;
        } else {
            return shared_ptr<Color>(new Color(color));
        }
    }

    method Tinter(const Color &color) :
        _tintColor(color)
    {
    }
    
    void method paint(const NamedImage &image) const
    {
        shared_ptr<NamedImage> result(new NamedImage(image));
        result->setTintColor(_tintColor);
        
        _result = result;
    }
    
    void method paint(const Color &color) const
    {
    }
    
    void method paint(const BitmapImage &image) const
    {
    }
    
    void method paint(const Gradient &gradient) const
    {
    }
}
