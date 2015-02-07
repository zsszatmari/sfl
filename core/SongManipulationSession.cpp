//
//  SongManipulationSession.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/20/13.
//
//

#include "SongManipulationSession.h"

namespace Gear
{
#define method SongManipulationSession::
    
    method SongManipulationSession(const shared_ptr<IApp> &app) :
        PlaylistSession(app)
    {
    }
    
    IExecutor & method executor()
    {
        return _executor;
    }
}