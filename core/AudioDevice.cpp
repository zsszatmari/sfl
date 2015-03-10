//
//  AudioDevice.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/5/13.
//
//

#include "AudioDevice.h"

namespace Gear
{
#define method AudioDevice::
    
    method AudioDevice(const long long deviceId, const string deviceName, const shared_ptr<const IPlaybackWorker> &target) :
        _deviceId(deviceId),
        _deviceName(deviceName),
        _target(target),
        _lastSeen(CHRONO_NS::system_clock::now())
    {
    }
    
    string method name() const
    {
        return _deviceName;
    }
    
    long long method deviceId() const
    {
        return _deviceId;
    }
    
    shared_ptr<const IPlaybackWorker> method target() const
    {
        return _target;
    }
    
    bool method operator==(const AudioDevice &rhs) const
    {
        return _target == rhs._target && _deviceId == rhs._deviceId;
    }

    bool method operator<(const AudioDevice &rhs) const
    {
        if (_target < rhs._target) {
            return true;
        }
        if (rhs._target < _target) {
            return false;
        }
        return _deviceId < rhs._deviceId;
    }

    int method lastSeenSecondsBefore() const
    {
        return CHRONO_NS::duration_cast<CHRONO_NS::milliseconds>(CHRONO_NS::system_clock::now() - _lastSeen).count() / 1000;
    }

    void method updateLastSeen()
    {
        _lastSeen = CHRONO_NS::system_clock::now();
    }
}