//
//  Writer.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 12/12/13.
//
//

#include "Writer.h"
#import "AppDelegate.h"

namespace Gui
{
#define method Writer::
    
    NSFont * method convertFont(TextAttributes::Font font)
    {
        if (yosemiteOrBetter()) {
            switch(font) {
                case TextAttributes::Font::Category:
                    return [NSFont systemFontOfSize:9.8f];
                case TextAttributes::Font::Title:
                    // title in tableview
                    return [NSFont systemFontOfSize:11.0];
                case TextAttributes::Font::Subtitle:
                    return [NSFont systemFontOfSize:11.0f];
                case TextAttributes::Font::Playing:
                    return [NSFont boldSystemFontOfSize:12.0f];
                case TextAttributes::Font::PlayingSub:
                    return [NSFont systemFontOfSize:14.0f];
                case TextAttributes::Font::Footnote: // time elapsed & remaining
                    return [NSFont systemFontOfSize:10.0f];
            }
        } else {
            switch(font) {
                case TextAttributes::Font::Category:
                    return [NSFont fontWithName:@"Helvetica" size:9.8f];
                case TextAttributes::Font::Title:
                    // title in tableview
                    return [NSFont systemFontOfSize:11.0];
                case TextAttributes::Font::Subtitle:
                    return [NSFont fontWithName:@"Helvetica" size:11.0f];
                case TextAttributes::Font::Playing:
                    return [NSFont fontWithName:@"Helvetica-Bold" size:12.0f];
                case TextAttributes::Font::PlayingSub:
                    return [NSFont fontWithName:@"Helvetica" size:14.0f];
                case TextAttributes::Font::Footnote: // time elapsed & remaining
                    return [NSFont fontWithName:@"HelveticaNeue" size:9.5f];
            }
        }
        return nil;
    }
    
    NSFont * method systemFont(float size, bool bold)
    {
        if (yosemiteOrBetter()) {
            if (bold) {
                return [NSFont boldSystemFontOfSize:size];
            } else {
                return [NSFont systemFontOfSize:size];
            }
        } else {
            if (bold) {
                return [NSFont fontWithName:@"HelveticaNeue-Bold" size:size];
            } else {
                return [NSFont fontWithName:@"HelveticaNeue" size:size];
            }
        }
    }
}