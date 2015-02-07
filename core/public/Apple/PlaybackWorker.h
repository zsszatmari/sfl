//
//  PlaybackWorker.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/8/13.
//
//

#ifndef __G_Ear_Player__PlaybackWorker__
#define __G_Ear_Player__PlaybackWorker__

#include <CoreAudio/CoreAudioTypes.h>
#include "IPlaybackWorker.h"
#include "Environment.h"
#if !TARGET_OS_IPHONE
#include <AudioToolbox/AudioToolbox.h>
#endif
#include MUTEX_H
#include SHAREDFROMTHIS_H
#include ATOMIC_H

namespace Gear
{
    class PlaybackWorker : public IPlaybackWorker, public MEMORY_NS::enable_shared_from_this<PlaybackWorker>
    {
    public:
        static shared_ptr<PlaybackWorker> create();
        
        virtual void setController(const shared_ptr<PlaybackController> &controller);
        virtual ~PlaybackWorker();
        virtual void play();
        virtual void stop(bool release);
#if TARGET_OS_MACDESKTOP
        bool resetVolume();
        void selectDeviceId(unsigned long long deviceId);
#endif
        virtual void setVolume(float volume);
        static AudioStreamBasicDescription usedPCMFormat();
        virtual PcmFormat pcmFormat();
        virtual vector<AudioDevice> availableDevices() const;
        virtual void setDevices(const vector<AudioDevice> &devices);
        
        uint32_t multiDevice() const; 
        std::vector<SInt32> multiChannelMap() const;
        
    private:
        PlaybackWorker();
        
#if TARGET_OS_MACDESKTOP
        OSStatus createAggregate();
        OSStatus setAggregateDevices(const std::vector<uint32_t> &deviceIds);
#endif
        
        vector<uint32_t> _sources;
        mutable THREAD_NS::mutex _sourcesMutex;
        mutable ATOMIC_NS::atomic<uint32_t> _multiDevice;
        
        std::vector<SInt32> _multiChannelMap;
        mutable THREAD_NS::mutex _multiChannelMapMutex;

        
        void *impl;
    };
}

#endif /* defined(__G_Ear_Player__PlaybackWorker__) */
