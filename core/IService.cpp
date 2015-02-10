//
//  IService.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#include "IService.h"

namespace Gear
{
#define method IService::
    
    method IService() :
        _state(IService::State::Offline)
    {
    }
    
    method ~IService()
    {
    }
    
    ValueConnector<IService::State> method stateConnector()
    {
        return _state.connector();
    }
    
    IService::State method state() const
    {
        return _state;
    }
    
    bool method disabled() const
    {
        return false;
    }
}