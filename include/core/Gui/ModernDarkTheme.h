//
//  ModernDarkTheme.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 06/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ModernDarkTheme__
#define __G_Ear_core__ModernDarkTheme__

#include "BlueTheme.h"

namespace Gui
{
    class ModernDarkTheme : public BlueTheme
    {
    public:
    	ModernDarkTheme();
        virtual TextAttributes miniplayerSubSubtitleText() const;
        virtual std::string name() const;
        virtual std::string noArtName() const override;
        
    private:
        virtual Color selected() const;
        virtual Color gray() const;
    };
}

#endif /* defined(__G_Ear_core__ModernDarkTheme__) */
