//
//  PlaybackController.h
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/7/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "stdplus.h"
#include "ISong.h"
#include "SerialExecutor.h"
#include "PlaybackItemDelegate.h"
#include "AtomicMutablePtr.h"
#include "IAudioGenerator.h"
#include SHAREDFROMTHIS_H
using MEMORY_NS::weak_ptr;

namespace Gear
{
    class IPlaybackWorker;
    class PlaybackControllerDelegate;
    class PlaybackItem;
    class PlaybackData;
    
    class core_export PlaybackController final : public MEMORY_NS::enable_shared_from_this<PlaybackController>, public PlaybackItemDelegate, public IAudioGenerator
    {
    public:
        static shared_ptr<PlaybackController> instance();
    
        shared_ptr<ISong> songPlayed();
        bool playSong(shared_ptr<ISong> song);
        void cacheSong(shared_ptr<ISong> song, shared_ptr<IPlaybackData> aData);
        void setDelegate(shared_ptr<PlaybackControllerDelegate> delegate);
        bool restartLastSong();
        void setRatio(float ratio);
        bool currentFinishedDownloading();
        
        void setVolume(float volume);
#if TARGET_OS_MACDESKTOP
        bool resetVolume();
#endif
        void setRateIfPlaying(float rate);
        
        virtual int getUncompressedDataInto(char *dataPtr, int length, const uint8_t bufferId);
        static void updatePlayCount(const shared_ptr<ISong> &song);
        
    private:
        static shared_ptr<PlaybackController> create();
        PlaybackController();
        
        virtual void rateChangedForPlaybackItem(const PlaybackItem &aItem, float rate);
        
        void pauseSong();
        
        bool _wasPlaying;
        void songFinished();
        void setAudio(shared_ptr<PlaybackItem> value);
        shared_ptr<PlaybackItem> audio();
        void updateTime();
        float total();
        void setData(shared_ptr<Gear::IPlaybackData> data);
        shared_ptr<PlaybackItem> audioWithData(shared_ptr<Gear::IPlaybackData> data);
        
        THREAD_NS::recursive_mutex _songMutex;
        Base::SerialExecutor _timer;
        bool _alreadyToldAboutFormat;
        
        AtomicMutablePtr<PlaybackItem> _audio;
        shared_ptr<PlaybackItem> _cachedAudio;
        shared_ptr<PlaybackItem> _restartAudio;
        
        shared_ptr<ISong> _restartSong;
        shared_ptr<ISong> _cachedSong;
        shared_ptr<ISong> _songPlayed;
        weak_ptr<ISong> _pausedSong;
        
        float _cachedDuration;
        shared_ptr<ISong> _cachedDurationForSongPlayed;
        
        shared_ptr<IPlaybackWorker> _playbackWorker;
        shared_ptr<PlaybackControllerDelegate> _delegate;
        
        friend class Player;
    };
}

