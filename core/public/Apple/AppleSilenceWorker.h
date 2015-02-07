#ifndef APPLESILENCEWORKER_H
#define APPLESILENCEWORKER_H

#include <AudioToolbox/AudioToolbox.h>
#include "IPlaybackWorker.h"

namespace Gear
{
	class AppleSilenceWorker : public IPlaybackWorker, public MEMORY_NS::enable_shared_from_this<AppleSilenceWorker>
	{
	public:
		static shared_ptr<AppleSilenceWorker> create();

        virtual ~AppleSilenceWorker();

        virtual vector<AudioDevice> availableDevices() const;
        static long long deviceId();

        virtual void play();
        virtual void stop(bool release = true);
        virtual void setVolume(float volume);

    private:
        AppleSilenceWorker();
        
        AudioQueueRef _queue;
        AudioQueueBufferRef _buffer;
	};
}

#endif