//
//  ModernDarkTheme.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 06/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <string>
#include "ModernDarkTheme.h"

namespace Gui
{
#define method ModernDarkTheme::
    
    method ModernDarkTheme()
    {
        _style = Style(R"(
            {
                "volumeslider": {
                    "knob": {
                        "osx": "regular",
                        "image": "slider-dark-knob"
                    }
                },
                "playlistpanel": {
                    "playlistlist": {},
                    "nowplayingtitle": {
                        "margin-top": 6,
                        "margin-bottom": 6 
                    },
                    "nowplayingimage": {}
                }
            }
        )");
    }

    Color method gray() const
    {
        return Color(207.0/255.0f, 207.0f/255.0f, 207.0f/255.0f);
    }
    
    Color method selected() const
    {
        return Color(63.0/255.0f, 101.0f/255.0f, 178.0f/255.0f);
    }
    
    TextAttributes method miniplayerSubSubtitleText() const
    {
        auto ret = BlueTheme::miniplayerSubSubtitleText();
        ret.setColor(Color(1.0f,1.0f,1.0f, 0.5f));
        
        return ret;
    }

    std::string method name() const
    {
        return "Modern Dark";
    }

    std::string method noArtName() const
    {
        return "noart";
    }
}
