//
//  CastPlaybackWorker.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 14/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__CastPlaybackWorker__
#define __G_Ear_core__CastPlaybackWorker__

#include "IPlaybackWorker.h"
#include "AtomicPtr.h"
#include <boost/asio.hpp>
#include SHAREDFROMTHIS_H
#include ATOMIC_H

namespace Cast
{
    class CastDevice;
    class Streamer;
}

namespace Gear
{
    class CastPlaybackWorker : public IPlaybackWorker, public MEMORY_NS::enable_shared_from_this<CastPlaybackWorker>
    {
    public:
        static shared_ptr<CastPlaybackWorker> create();
        
        virtual void play();
        virtual void stop(bool release = true);
        virtual void setVolume(float volume);
        
        virtual vector<AudioDevice> availableDevices() const;
        virtual void setDevices(const vector<AudioDevice> &devices);
        virtual int affinity() const;
        virtual void terminate();

        void deviceDown(const string &ip);
        
    private:
        CastPlaybackWorker();
        
        void foundDevice(const shared_ptr<Cast::CastDevice> &device);
        AudioDevice convertDevice(const Cast::CastDevice &device);
        void connect(const shared_ptr<Cast::CastDevice> &device);
        
        void probe();
        void init();
        
        ATOMIC_NS::atomic<float> _volume;
        ATOMIC_NS::atomic_bool _playing;
        
        boost::asio::deadline_timer _probeTimer;
        shared_ptr<Cast::Streamer> _streamer;
        
        vector<shared_ptr<Cast::CastDevice>> _activeDevices;
        AtomicPtr<vector<AudioDevice>> _devices;
    };
}

#endif /* defined(__G_Ear_core__CastPlaybackWorker__) */
