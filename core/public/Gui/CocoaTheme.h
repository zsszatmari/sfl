//
//  CocoaTheme.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 31/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__CocoaTheme__
#define __G_Ear_core__CocoaTheme__

#include "ITheme.h"
#include "BlueTheme.h"

namespace Gui
{
    class CocoaTheme : public BlueTheme
    {
    public:
    	CocoaTheme();

        ITheme::SliderLook volumeSlider() const;
        virtual std::string name() const;

    	virtual std::string noArtName() const;
    };
}

#endif /* defined(__G_Ear_core__CocoaTheme__) */
