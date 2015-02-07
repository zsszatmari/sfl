//
//  PlayerBase.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/7/13.
//
//

#ifndef __G_Ear_Player__PlayerBase__
#define __G_Ear_Player__PlayerBase__

#include "IPlayer.h"
#include "SongEntry.h"
#include "SongGrouping.h"
#include "ManagedValue.h"
#include "PlaybackControllerDelegate.h"
#include "Interruptor.h"
#include "SerialExecutor.h"
#include CHRONO_H
#include SHAREDFROMTHIS_H
#include ATOMIC_H

namespace Gear
{
    class IPlaylist;
    class ISongArray;
    
    class PlayerBase : public IPlayer, public MEMORY_NS::enable_shared_from_this<PlayerBase>, public PlaybackControllerDelegate
    {
    public:
        virtual void play(const SongEntry &song); 
        virtual void play(const shared_ptr<IPlaylist> &playlist, const SongEntry &song, bool forceRestart = false);
        virtual void next();
        virtual void prev();
        virtual void play() = 0;
        virtual void rate(const shared_ptr<ISong> &song);
        virtual void rate(const shared_ptr<ISong> &song, int selected);
        virtual void rate();
        virtual void setRatio(float ratio);
        
    protected:
        PlayerBase();
        void init();
        void songFinished();
        
    private:
        bool next(bool forward);

        virtual void play(const shared_ptr<ISong> &song);
        virtual bool restartLastSong() = 0;
        virtual void doSetRatio(float ratio) = 0;
        virtual void playTimePassed(float duration) = 0;
        virtual bool currentFinishedDownloading() = 0;
        virtual void cacheSong(const shared_ptr<ISong> &song, shared_ptr<IPlaybackData> data) = 0;
        void shuffleChanged(const bool change);
        void repeatChanged(const Repeat change);
        void ratingChanged(const int change);
        
        virtual void setElapsedTime(float elapsed);
        virtual void setRemainingTime(float remaining);
        virtual void changedRatio(float ratio);
        virtual void songFinished(shared_ptr<Gear::ISong> songPlayed);
        
        SongEntry _cachedSong;
        shared_ptr<ISongArray> _arrayForJumping;
        shared_ptr<ISong> _lastPlayedSong;
        bool _scheduling;
        float _lastElapsedTime;
        float _lastRemainingTime;
        bool _shuffleEnabledSinceSchedule;
        CHRONO_NS::steady_clock::time_point _lastElapsedTimeStamp;
        ATOMIC_NS::atomic<uint64_t> _lastNetworkFailure;
        bool _delegateSet;
        
        void updateNextSong();
        void scheduleUpdateNextSong();
        bool songValid(const SongEntry &entry) const;
        void clearSong();
        
        Base::SerialExecutor _playExecutor;
        THREAD_NS::mutex _playInterruptMutex;
        shared_ptr<Interruptor::Handle> _playInterruptHandle;

    };
}

#endif /* defined(__G_Ear_Player__PlayerBase__) */
