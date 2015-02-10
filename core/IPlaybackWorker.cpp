//
//  IPlaybackWorker.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/8/13.
//
//

#include "IPlaybackWorker.h"

namespace Gear
{
#define method IPlaybackWorker::
    
    method IPlaybackWorker()
    {
    }
    
    method ~IPlaybackWorker()
    {
    }
    
    shared_ptr<PlaybackController> method controller() const
    {
        return _controller;
    }
    
    void method setController(const shared_ptr<PlaybackController> &controller)
    {
        _controller = controller;
    }
    
    void method setDevices(const vector<AudioDevice> &devices)
    {
        _devices = devices;
    }

    vector<AudioDevice> method devices()
    {
        return _devices;
    }
    
    bool method resetVolume()
    {
        return false;
    }
    
    IPlaybackWorker::PcmFormat method pcmFormat()
    {
        return PcmFormat::Float32;
    }
    
    int method affinity() const
    {
        return 0;
    }
    
    void method setGenerator(const shared_ptr<IAudioGenerator> &generator)
    {
        _generator = generator;
    }
    
    shared_ptr<IAudioGenerator> method generator() const
    {
        return _generator;
    }

    void method terminate()
    {
    }

    void method reload()
    {
    }
}