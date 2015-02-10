//
//  GooglePlayNonLocalArray.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#include "GooglePlayNonLocalArray.h"

namespace Gear
{
#define method GooglePlayNonLocalArray::
    
    method GooglePlayNonLocalArray(const shared_ptr<ISession> &session, const ClientDb::Predicate &predicate) :
        NonLocalArray(session, predicate)
    {
    }
    
    const weak_ptr<GooglePlaySession> method session() const
    {
        return MEMORY_NS::static_pointer_cast<GooglePlaySession>(SessionSongArray::session().lock());
    }
}