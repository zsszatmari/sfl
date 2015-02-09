//
//  ISongIntent.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#include "ISongIntent.h"

namespace Gear
{
#define method ISongIntent::
     
    method ~ISongIntent()
    {
    }
    
    bool method confirmationNeeded(const vector<SongEntry> &songs) const
    {
        return false;
    }
    
    string method confirmationText(const vector<SongEntry> &songs) const
    {
        return "";
    }
}