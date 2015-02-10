//
//  TextAttributes.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__TextAttributes__
#define __G_Ear_core__TextAttributes__

#include "Color.h"

namespace Gui
{
    class core_export TextAttributes final
    {
    public:
        enum class Font
        {
            Category,
            Title,
            Subtitle,
            Playing,
            PlayingSub,
            Footnote
        };
        
        TextAttributes();
        TextAttributes(const Font font);
        TextAttributes(const Font font, const Color &color);
        
        const Font font() const;
        const Color &color() const;
        void setColor(const Color &color);
        
    private:
        Font _font;
        Color _color;
    };
}

#endif /* defined(__G_Ear_core__TextAttributes__) */
