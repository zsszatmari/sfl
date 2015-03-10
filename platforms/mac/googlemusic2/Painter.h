//
//  Painter.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__Painter__
#define __G_Ear_Player__Painter__

#import <Foundation/Foundation.h>
#include "IPainter.h"

namespace Gui
{
    class IPaintable;
    
    class Painter : public IPainter
    {
    public:
        static void paint(const IPaintable &paintable, CGRect frame);
        static NSImage *convertImage(const IPaintable &paintable);
        static NSColor *convertColor(const Color &color);
        
      private:
        Painter(CGRect frame);
        
        virtual void paint(const Color &color) const;
        virtual void paint(const NamedImage &paintable) const;
        virtual void paint(const BitmapImage &image) const;
        virtual void paint(const Gradient &gradient) const;
        
        CGRect _frame;
    };
}

#endif /* defined(__G_Ear_Player__Painter__) */
