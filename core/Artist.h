//
//  Artist.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#ifndef __G_Ear__Artist__
#define __G_Ear__Artist__

#include "IArtist.h"

namespace Gear
{
    class Artist : public IArtist
    {
    public:
        Artist(const string uniqueId);
        virtual const string uniqueId() const;
        
    private:
        const string _uniqueId;
    };
}

#endif /* defined(__G_Ear__Artist__) */
