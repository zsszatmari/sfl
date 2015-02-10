//
//  PhoneCategoriesTransformer.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__PhoneCategoriesTransformer__
#define __G_Ear_core__PhoneCategoriesTransformer__

#include "ICategoriesTransformer.h"

namespace Gear
{
    class PhoneCategoriesTransformer final : public ICategoriesTransformer
    {
    public:
        virtual vector<PlaylistCategory> transform(const vector<PlaylistCategory> &) const;
    };
}

#endif /* defined(__G_Ear_core__PhoneCategoriesTransformer__) */
