//
//  NamedImage.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__NamedImage__
#define __G_Ear_core__NamedImage__

#include <string>
#include "stdplus.h"
#include "IPaintable.h"
#include "Color.h"

namespace Gui
{
    using std::string;
    
    class Color;
    
    class core_export NamedImage : public IPaintable
    {
    public:
        NamedImage(const string &name, const Color &tintColor = Color());
        
        virtual void paint(const IPainter &painter) const;
        
        //NamedImage(const NamedImage &rhs);
        virtual string imageName() const;
        const Color &tintColor() const;
        void setTintColor(const Color &tintColor);
        
    private:
        string _imageName;
        Color _tintColor;
    };
}

#endif /* defined(__G_Ear_core__NamedImage__) */
