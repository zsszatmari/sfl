//
//  Album.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#include "Album.h"
#include "Environment.h"

namespace Gear
{
    using std::string;
    
#define method Album::
    
    method Album(const string uniqueId) :
        _uniqueId(uniqueId)
    {
    }
    
    const string method uniqueId() const
    {
        return _uniqueId;
    }
    
    bool method operator==(const IAlbum &rhs) const
    {
        const Album *pRhs = dynamic_cast<const Album *>(&rhs);
        if (pRhs == nullptr) {
            return false;
        }
        return (_uniqueId == pRhs->_uniqueId);
    }
}
