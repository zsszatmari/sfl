//
//  IAudioDeviceEnumerator.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 12/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__IAudioDeviceEnumerator__
#define __G_Ear_core__IAudioDeviceEnumerator__

#include <vector>
#include "AudioDevice.h"

namespace Gear
{
    using std::vector;
    
    class core_export IAudioDeviceEnumerator
    {
    public:
        virtual ~IAudioDeviceEnumerator();
        virtual vector<AudioDevice> availableDevices() const;
    };
}

#endif /* defined(__G_Ear_core__IAudioDeviceEnumerator__) */
