//
//  Artist.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#include "Artist.h"

namespace Gear
{
#define method Artist::
    
    method Artist(const string uniqueId) :
        _uniqueId(uniqueId)
    {
    }
    
    const string method uniqueId() const
    {
        return _uniqueId;
    }
}