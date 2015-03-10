//
//  Writer.mm
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "Writer.h"
#include "TextAttributes.h"
#include "Painter.h"

//#define DISABLE_IOS7_TYPOGRAPHY
#define USE_OWN_SIZES

namespace Gui
{
#define method Writer::
    
    static UIFont *getFont(TextAttributes::Font font)
    {
        switch(font) {
            case TextAttributes::Font::Category:
                return [UIFont fontWithName:@"Helvetica" size:9.8f];
            case TextAttributes::Font::Title:
                return [UIFont fontWithName:@"Helvetica-Bold" size:14.5f];
            case TextAttributes::Font::Subtitle:
                return [UIFont fontWithName:@"Helvetica" size:11.0f];
            case TextAttributes::Font::Playing:
                return [UIFont fontWithName:@"Helvetica" size:15.0f];
            case TextAttributes::Font::PlayingSub:
                return [UIFont fontWithName:@"Helvetica" size:12.0f];
            case TextAttributes::Font::Footnote:
                return [UIFont fontWithName:@"Helvetica" size:11.0f];
		default:
                return [UIFont fontWithName:@"Helvetica" size:11.0f];
        }
    }
    
    static NSString *getTextStyle(TextAttributes::Font font)
    {
        switch(font) {
            case TextAttributes::Font::Category:
                return UIFontTextStyleFootnote;
            case TextAttributes::Font::Title:
                return UIFontTextStyleHeadline;
            case TextAttributes::Font::Subtitle:
                return UIFontTextStyleSubheadline;
            case TextAttributes::Font::Playing:
                return UIFontTextStyleHeadline;
            case TextAttributes::Font::PlayingSub:
                return UIFontTextStyleSubheadline;
            case TextAttributes::Font::Footnote:
                return UIFontTextStyleCaption2;
		default:
                return UIFontTextStyleFootnote;
        }
    }
    
    static UIFont *getEnhancedFont(TextAttributes::Font font)
    {
        return [UIFont preferredFontForTextStyle:getTextStyle(font)];
    }

    UIFont * method convertFont(const TextAttributes &attrib)
    {
        UIFont *font;
        
        BOOL ios7Enabled = NO;
#ifndef DISABLE_IOS7_TYPOGRAPHY
        ios7Enabled = YES;
#endif
        if (ios7Enabled && [[UIFont class] respondsToSelector:@selector(preferredFontForTextStyle:)]) {
            
#ifdef USE_OWN_SIZES
            UIFont *fontForSize = getFont(attrib.font());
            float fontSize = fontForSize.pointSize;
            font = [UIFont fontWithDescriptor:[UIFontDescriptor preferredFontDescriptorWithTextStyle:getTextStyle(attrib.font())] size:fontSize];
#else
            font = getEnhancedFont(attrib.font());
#endif
            
        } else {
            font = getFont(attrib.font());
        }
        return font;
    }
    
    void method apply(const TextAttributes &attrib, UILabel *label)
    {
        label.font = convertFont(attrib);
        
        if (attrib.color()) {
            label.textColor = Painter::convertColor(attrib.color());
        }
    }
}
