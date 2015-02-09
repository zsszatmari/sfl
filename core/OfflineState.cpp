//
//  OfflineState.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 30/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "OfflineState.h"

namespace Gear
{
#define method OfflineState::
    
    void method setOffline(bool offline, float ratio)
    {
        _offline = offline;
        _ratio = ratio;
    }
    
    Base::ValueConnector<bool> method offlineConnector()
    {
        return _offline.connector();
    }
    
    Base::ValueConnector<float> method ratioConnector()
    {
        return _ratio.connector();
    }
}
