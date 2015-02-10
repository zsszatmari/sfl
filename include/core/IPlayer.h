//
//  IPlayer.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__IPlayer__
#define __G_Ear_iOS__IPlayer__

#include "ISong.h"
#include "SongGrouping.h"
#include "ManagedValue.h"
#include "SongEntry.h"
#include "EventSignal.h"

namespace Gear
{
    using namespace Base;
    
    class IPlaylist;
    
    class core_export IPlayer
    {
    public:
        enum class Repeat
        {
            Off = 0,
            On  = 1,
            OneSong = -1
        };
        
        IPlayer();
        virtual ~IPlayer();
        virtual void play(const SongEntry &song) = 0;
        virtual void prev() = 0;
        virtual void play() = 0;
        virtual void next() = 0;
        virtual void rate(const shared_ptr<ISong> &song) = 0;
        virtual void rate(const shared_ptr<ISong> &song, int selected) = 0;
        virtual void rate() = 0;
        virtual void setRatio(float ratio) = 0;
        
        ValueConnector<float> songRatioConnector();
        ValueConnector<string> elapsedTimeConnector();
        ValueConnector<string> remainingTimeConnector();
        ValueConnector<SongEntry> songEntryConnector();
        ValueConnector<bool> playingConnector();
        EventConnector ratedConnector();
        shared_ptr<PromisedImage> albumArt(const long imageSize) const;
        virtual shared_ptr<IPlaylist> playlistCurrentlyPlaying() const;
        virtual ValueConnector<shared_ptr<IPlaylist>> playlistCurrentlyPlayingConnector();
        SongGrouping groupingCurrentlyPlaying() const;

        // shuffle, repeat: only client changes it
        ManagedValue<bool> &shuffle();
        ManagedValue<Repeat> &repeat();

        // rating: only core changes it
        ManagedValue<int> &rating();
        
    protected:
        Base::EventSignal _ratedEvent;

        ManagedValue<bool> _playing;
        ManagedValue<float> _songRatio;
        ManagedValue<string> _elapsedTime;
        ManagedValue<string> _remainingTime;
        ManagedValue<SongEntry> _songPlayed;
        ManagedValue<bool> _shuffle;
        ManagedValue<Repeat> _repeat;
        ManagedValue<int> _rating;
        
        SongGrouping _groupingCurrentlyPlaying;
        ManagedValue<shared_ptr<IPlaylist>> _playlistCurrentlyPlaying;
        
    private:
    };
};

#endif /* defined(__G_Ear_iOS__IPlayer__) */
