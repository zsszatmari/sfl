//
//  AudioDeviceManager.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/5/13.
//
//

#ifndef __G_Ear_Player__AudioDeviceManager__
#define __G_Ear_Player__AudioDeviceManager__

#include <vector>
#include "AudioDevice.h"
#include "stdplus.h"

namespace Gear
{
    using std::vector;
    
    class AudioDeviceManager final
    {
    public:
        static AudioDeviceManager &instance();
        
        vector<AudioDevice> availableDevices() const;
        long long defaultDeviceId() const;
        void setSources(const vector<uint32_t> &sources);
        vector<uint32_t> sources() const;
        static string aggregateName();
        
    private:
        AudioDeviceManager();
        AudioDeviceManager(const AudioDeviceManager &); // delete
        AudioDeviceManager &operator=(const AudioDeviceManager &); // delete
        
        vector<uint32_t> _sources;
        mutable mutex _sourcesMutex;
    };
}

#endif /* defined(__G_Ear_Player__AudioDeviceManager__) */
