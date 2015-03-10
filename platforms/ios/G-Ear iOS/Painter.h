//
//  Painter.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__Painter__
#define __G_Ear_iOS__Painter__

#include <functional>
#include "stdplus.h"
#include MEMORY_H
#include "IPainter.h"

namespace Gui
{
    class IPaintable;
    
    class Painter final : public IPainter
    {
    public:
        // must be called from UIView's drawRect
        static bool paint(const shared_ptr<IPaintable> &, bool stretch);
        static bool paint(const shared_ptr<IPaintable> &, bool stretch, CGRect rect);
        
        static UIImage *convertImage(const shared_ptr<IPaintable> &paintable, CGSize size);
        static UIColor *convertColor(const Color &color);
        
        virtual void paint(const Color &color) const;
        virtual void paint(const NamedImage &image) const;
        virtual void paint(const BitmapImage &image) const;
        virtual void paint(const Gradient &gradient) const;
        
    private:
        Painter(CGRect rect, bool stretch, const shared_ptr<IPaintable> &paintable);
        
        static UIImage *tint(UIImage *image, const Color &tintColor);
        
        const bool _stretch;
        const CGRect _rect;
        const MEMORY_NS::shared_ptr<IPaintable> _paintable;
        std::function<void()> _callIfCantConvert;
        mutable bool _success;
        
        bool _converting;
        mutable bool _wasConverted;
        mutable UIImage *_converted;
    };
}

#endif /* defined(__G_Ear_iOS__Painter__) */
