#pragma once

#include <dsound.h>
#include "IPlaybackWorker.h"


namespace Gear
{
	class PlaybackWorkerDX final : public IPlaybackWorker
	{
	public:
		PlaybackWorkerDX();
		virtual ~PlaybackWorkerDX();
                virtual void play() override;

                virtual void stop(bool release) override;

                virtual void setVolume(float volume) override;
                virtual PcmFormat pcmFormat() override;

                virtual vector<AudioDevice> availableDevices() const override;

	private:
		bool init();
		void fillBuffer();

		bool _playing;
		HANDLE _fillEvent;
		IDirectSound8 *_directSound;
		IDirectSoundBuffer *_primaryBuffer;
		IDirectSoundBuffer8 *_secondaryBuffer;
	};


}
