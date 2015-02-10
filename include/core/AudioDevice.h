//
//  AudioDevice.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/5/13.
//
//

#ifndef __G_Ear_Player__AudioDevice__
#define __G_Ear_Player__AudioDevice__

#include <string>
#include "stdplus.h"
#include CHRONO_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    using std::string;
    
    class IPlaybackWorker;
    
    class core_export AudioDevice final
    {
    public:
        AudioDevice(const long long deviceId, const string deviceName, const shared_ptr<const IPlaybackWorker> &target);
        string name() const;
        long long deviceId() const;
        shared_ptr<const IPlaybackWorker> target() const;
        bool operator==(const AudioDevice &) const;
        bool operator<(const AudioDevice &) const;

        int lastSeenSecondsBefore() const;
        void updateLastSeen();
        
    private:
        long long _deviceId;
        string _deviceName;
        shared_ptr<const IPlaybackWorker> _target;

        CHRONO_NS::system_clock::time_point _lastSeen; 
    };
}

#endif /* defined(__G_Ear_Player__AudioDevice__) */
