//
//  CocoaTheme.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 31/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "CocoaTheme.h"
#include "Gradient.h"
#include "NamedImage.h"

namespace Gui
{
#define method CocoaTheme::
    
    using std::vector;

    method CocoaTheme()
    {
        _style = Style(R"(
            {
                "general": {
                    "topbar":[[255,255,255],[224,224,224]],
                    "bottombar":[[224,224,224],[255,255,255]],
                    "text":[4,5,4]
                },
                "volumeslider": {
                    "knob": {
                        "osx": "regular",
                        "image": "slider-flat-knob"
                    },
                    "left": {
                        "image": "slider-flat-left"
                    },
                    "leftend": {
                        "image": "slider-flat-leftend"
                    },
                    "right": {
                        "image": "slider-flat-right"
                    },
                    "rightend": {
                        "image": "slider-flat-rightend"
                    }
                },
                "albumsizeslider": {
                    "osx": "mini"
                },
                "seekslider": {
                    "knob": {
                        "osx": "mini",
                        "image": "slider-flat-knob"
                    },
                    "left": {
                        "image": "slider-flat-left"
                    },
                    "leftend": {
                        "image": "slider-flat-leftend"
                    },
                    "right": {
                        "image": "slider-flat-right"
                    },
                    "rightend": {
                        "image": "slider-flat-rightend"
                    }
                },
                "playlistpanel": {
                    "playlistlist": {},
                    "nowplayingtitle": {
                        "margin-top": 8,
                        "margin-bottom": 13 
                    },
                    "nowplayingimage": {}
                }
            }
        )");
    }
    
    ITheme::SliderLook method volumeSlider() const
    {
        SliderLook look;
        look.thickness = 9.5f;
        //look.leftSideWidth = 1.0f;
        //look.rightSideWidth = 0.0f;
        //look.leftSide = center;
        look.thumbWidth = 16.0f;

        look.sideLength = 4;
        look.minimumTrack = shared_ptr<NamedImage>(new NamedImage("cocoaslidermin"));
        look.maximumTrack = shared_ptr<NamedImage>(new NamedImage("cocoaslidermax"));
        look.leftSide = shared_ptr<NamedImage>(new NamedImage("cocoasliderleft"));
        look.rightSide = shared_ptr<NamedImage>(new NamedImage("cocoasliderright"));
        look.thumb = shared_ptr<NamedImage>(new NamedImage("cocoasliderknob"));
        
        return look;
    }

    std::string method name() const
    {
        return "Cocoa";
    }

    string method noArtName() const
    {
        return "noart-cocoa";
    }
}