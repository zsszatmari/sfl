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
                        "padding-top": 6,
                        "padding-bottom": 6 
                    },
                    "nowplayingimage": {}
                },
                "controls": {
                    "play": {
                        "normal": "darkmodern_play_bezel",
                        "pushed": "darkmodern_play_bezel_pushed",
                        "margin-left": 3,
                        "margin-right": 3
                    },
                    "pause": {
                        "normal": "darkmodern_play_bezel",
                        "pushed": "darkmodern_play_bezel_pushed",
                        "margin-left": 3,
                        "margin-right": 3    
                    },
                    "prev": {
                        "normal": "darkmodern_next_bezel",
                        "pushed": "darkmodern_next_bezel_pushed",
                        "margin-left": 73,
                        "margin-right": 3
                    },
                    "next": {
                        "normal": "darkmodern_next_bezel",
                        "pushed": "darkmodern_next_bezel_pushed",
                        "margin-left": 3,
                        "margin-right": 3
                    }
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
