//
//  IConceiver.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/28/13.
//
//

#include "IConceiver.h"

namespace Gear
{
    
#define method IConceiver::
    
    method IConceiver(const weak_ptr<const ISession> &session)
        : _session(session)
    {
    }
    
    method ~IConceiver()
    {
    }
    
    const weak_ptr<const ISession> method session() const
    {
        return _session;
    }
    
#undef method
}