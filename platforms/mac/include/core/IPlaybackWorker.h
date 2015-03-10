//
//  IPlaybackWorker.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/8/13.
//
//

#ifndef __G_Ear_Player__IPlaybackWorker__
#define __G_Ear_Player__IPlaybackWorker__

#include "stdplus.h"
#include "IAudioDeviceEnumerator.h"

#define ENABLE_UNIVERSAL_EQ

namespace Gear
{
    class PlaybackController;
    class IAudioGenerator;
     
    class core_export IPlaybackWorker : public IAudioDeviceEnumerator
    {
    public:
        IPlaybackWorker();
        virtual ~IPlaybackWorker();
        virtual void play() = 0;
        virtual void stop(bool release = true) = 0;
        virtual void setVolume(float volume) = 0;
        void setGenerator(const shared_ptr<IAudioGenerator> &generator);
        shared_ptr<IAudioGenerator> generator() const;
        
        shared_ptr<PlaybackController> controller() const;
        virtual void setController(const shared_ptr<PlaybackController> &);
        
        enum class PcmFormat
        {
            Signed16,
            Float32
        };

        // declared in all PlaybackWorker implementations:
		virtual PcmFormat pcmFormat();
        virtual void setDevices(const vector<AudioDevice> &devices);
        virtual vector<AudioDevice> devices();
        virtual vector<AudioDevice> availableDevices() const = 0;
        virtual void terminate();

        // how much does it want to be the master
        virtual int affinity() const;
        
        // makes sense only on desktop
        virtual bool resetVolume();

        virtual void reload();

    protected:
        shared_ptr<IAudioGenerator> _generator;
        
    private:
        IPlaybackWorker &operator=(const IPlaybackWorker &); //delete
        IPlaybackWorker(const IPlaybackWorker &); // delete
        
        shared_ptr<PlaybackController> _controller;
        vector<AudioDevice> _devices;
    };
}

#endif /* defined(__G_Ear_Player__IPlaybackWorker__) */