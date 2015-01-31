#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include "IPlaybackWorker.h"

namespace Gear
{
    class PlaybackWorker : public IPlaybackWorker
    {
    public:
        PlaybackWorker();

        virtual void play();
        virtual void stop();
        virtual void setVolume(float volume);
        virtual PcmFormat pcmFormat();
    };
}

#endif // PLAYBACKWORKER_H
