//
//  ICategoriesTransformer.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ICategoriesTransformer__
#define __G_Ear_core__ICategoriesTransformer__

#include "PlaylistCategory.h"

namespace Gear
{
    class ICategoriesTransformer
    {
    public:
        virtual ~ICategoriesTransformer();
        virtual vector<PlaylistCategory> transform(const vector<PlaylistCategory> &) const = 0;
    };
}

#endif /* defined(__G_Ear_core__ICategoriesTransformer__) */
