//
//  Tinter.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__Tinter__
#define __G_Ear_core__Tinter__

#include "IPainter.h"
#include "stdplus.h"
#include "Color.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gui
{
    class IPaintable;
    class NamedImage;
    
    class Tinter final : public IPainter
    {
    public:
        static shared_ptr<IPaintable> tintWithColor(const shared_ptr<IPaintable> &icon, const Color &color);
        
        virtual void paint(const Color &color) const;
        virtual void paint(const NamedImage &image) const;
        virtual void paint(const BitmapImage &image) const;
        virtual void paint(const Gradient &image) const;
        
    private:
        Tinter(const Color &color);
        
        Color _tintColor;
        mutable shared_ptr<IPaintable> _result;
    };
}

#endif /* defined(__G_Ear_core__Tinter__) */
