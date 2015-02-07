//
//  IAudioDeviceEnumerator.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 12/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "IAudioDeviceEnumerator.h"

namespace Gear
{
#define method IAudioDeviceEnumerator::
    
    method ~IAudioDeviceEnumerator()
    {
    }
    
    vector<AudioDevice> method availableDevices() const
    {
        return vector<AudioDevice>();
    }
}
