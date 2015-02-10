//
//  AllAccessToLibraryIntent.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#include <iostream>
#include "AllAccessToLibraryIntent.h"
#include "GearUtility.h"

namespace Gear {

#define method AllAccessToLibraryIntent::
    
    method AllAccessToLibraryIntent(const shared_ptr<ISession> &session) :
        _session(session)
    {
    }
    
    const string method menuText() const
    {
        return "Add to Library";
    }
    
    void method apply(const vector<shared_ptr<ISong>> &songs) const
    {
        auto localSession = _session.lock();
        if (localSession) {
            localSession->addAllAccessToLibrary(songs);
        }
    }
}