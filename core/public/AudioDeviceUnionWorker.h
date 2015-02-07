//
//  AudioDeviceUnionEnumerator.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 13/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__AudioDeviceUnionEnumerator__
#define __G_Ear_core__AudioDeviceUnionEnumerator__

#include "IPlaybackWorker.h"
#include "stdplus.h"
#include SHAREDFROMTHIS_H
#include "AtomicPtr.h"
#include "IAudioGenerator.h"
#include "CircularBuffer.h"

namespace Gear
{
    class IEqualizer;

    class AudioDeviceUnionWorker final : public IPlaybackWorker, public IAudioGenerator, public MEMORY_NS::enable_shared_from_this<AudioDeviceUnionWorker>
    {
    public:
        static shared_ptr<AudioDeviceUnionWorker> create(const shared_ptr<IPlaybackWorker> &firstDefault, const shared_ptr<IPlaybackWorker> &second);
        static shared_ptr<AudioDeviceUnionWorker> create(const vector<shared_ptr<IPlaybackWorker>> workers);
        
        virtual vector<AudioDevice> availableDevices() const;
        virtual void setDevices(const vector<AudioDevice> &devices);
        virtual vector<AudioDevice> devices();
        
        virtual void play();
        virtual void stop(bool release = true) ;
        virtual void setVolume(float volume);
        virtual PcmFormat pcmFormat();
        virtual int getUncompressedDataInto(char *dataPtr, int length, uint8_t bufferId);
        virtual void silence(uint8_t bufferId);
        virtual void terminate();
        virtual bool resetVolume();

        virtual void reload();
        
    private:
        AudioDeviceUnionWorker(const vector<shared_ptr<IPlaybackWorker>> enumerators);
        
        AtomicPtr<vector<shared_ptr<IPlaybackWorker>>> _activeWorkers;
        static const uint8_t kBufferNum = 2;
        CircularBufferSized<44100> _buffers[kBufferNum];
        ATOMIC_NS::atomic_bool _active[kBufferNum];

        ATOMIC_NS::atomic_bool _playing;
        const shared_ptr<IEqualizer> _equalizer;
        
        void init();
        void initEq(size_t bands);
        void processEq(char *data, size_t num);

        const vector<shared_ptr<IPlaybackWorker>> _workers;
    };
}

#endif /* defined(__G_Ear_core__AudioDeviceUnionEnumerator__) */
