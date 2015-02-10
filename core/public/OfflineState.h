//
//  OfflineState.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 30/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__OfflineState__
#define __G_Ear_core__OfflineState__

#include "ManagedValue.h"

namespace Gear
{
    class OfflineState final
    {
    public:
        void setOffline(bool offline, float ratio);
        Base::ValueConnector<bool> offlineConnector();
        Base::ValueConnector<float> ratioConnector();
        
    private:
        Base::ManagedValue<bool> _offline;
        Base::ManagedValue<float> _ratio;
    };
}

#endif /* defined(__G_Ear_core__OfflineState__) */
