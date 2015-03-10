//
//  Writer.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__Writer__
#define __G_Ear_iOS__Writer__

namespace Gui
{
    class TextAttributes;
    
    class Writer final
    {
    public:
        static void apply(const TextAttributes &attrib, UILabel *label);
        static UIFont *convertFont(const TextAttributes &attrib);
        
    private:
        Writer(); // delete
    };
}

#endif /* defined(__G_Ear_iOS__Writer__) */
