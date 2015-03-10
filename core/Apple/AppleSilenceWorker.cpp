#include "AppleSilenceWorker.h"
#include "sfl/Vector.h"

namespace Gear
{
#define method AppleSilenceWorker::

	shared_ptr<AppleSilenceWorker> method create()
	{
        return shared_ptr<AppleSilenceWorker>(new AppleSilenceWorker());
	}

	method AppleSilenceWorker() :
        _queue(nullptr),
        _buffer(nullptr)
	{
	}
    
    method ~AppleSilenceWorker()
    {
        if (_queue) {
            if (_buffer) {
                AudioQueueFreeBuffer(_queue,_buffer);
            }
            AudioQueueDispose(_queue, false);
        }
    }
    
    static void emptyCallback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
    {
        memset(inBuffer->mAudioData, 0, inBuffer->mAudioDataBytesCapacity);
        inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity;
        AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
    }

    void method play()
    {
        if (!_queue) {
            AudioStreamBasicDescription format;
            format.mChannelsPerFrame = 2;
            format.mSampleRate = 44100;
            format.mFormatID = kAudioFormatLinearPCM;
            format.mBitsPerChannel = 32;
            format.mFormatFlags = kAudioFormatFlagIsFloat;
            format.mFramesPerPacket = 1;
            format.mBytesPerFrame = format.mBitsPerChannel / 8 * format.mChannelsPerFrame;
            format.mBytesPerPacket = format.mFramesPerPacket * format.mBytesPerFrame;
            
            AudioQueueNewOutput(&format, &emptyCallback, nullptr, NULL, NULL, 0, &_queue);
        }
        if (_queue) {
            AudioQueueStart(_queue, NULL);
            if (!_buffer) {
                AudioQueueAllocateBuffer(_queue, 512, &_buffer);
            }
            emptyCallback(nullptr, _queue, _buffer);
        }
    }

    void method stop(bool release)
    {
        if (_queue && release) {
            AudioQueueStop(_queue, false);
        }
    }

    void method setVolume(float volume)
    {
    }

    long long method deviceId()
    {
        return -1;
    }
    
    vector<AudioDevice> method availableDevices() const
    {
        return sfl::Vector::singleton(AudioDevice(deviceId(), "Silence", shared_from_this()));
    }

}