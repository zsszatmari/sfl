//
//  Writer.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 12/12/13.
//
//

#ifndef __G_Ear_Player__Writer__
#define __G_Ear_Player__Writer__

#import <Foundation/Foundation.h>
#include "TextAttributes.h"

namespace Gui
{
    class Writer
    {
    public:
        static NSFont *convertFont(TextAttributes::Font font);
        static NSFont *systemFont(float size, bool bold);
    };
}

#endif /* defined(__G_Ear_Player__Writer__) */
