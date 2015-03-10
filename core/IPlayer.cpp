    //
//  IPlayer.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "IPlayer.h"
#include "IApp.h"
#include "AlbumArtStash.h"
#include "ISession.h"
#include "Tracking.h"
#include "IPlaylist.h"

namespace Gear
{
#define method IPlayer::
    
    method IPlayer() :
        _songRatio(0),
        _playing(false),
        _shuffle(false),
        _rating(0),
        _repeat(Repeat::Off)
    {
    }
    
    method ~IPlayer()
    {
    }
    
    ValueConnector<float> method songRatioConnector()
    {
        return _songRatio.connector();
    }
    
    ValueConnector<string> method remainingTimeConnector()
    {
        return _remainingTime.connector();
    }
    
    ValueConnector<string> method elapsedTimeConnector()
    {
        return _elapsedTime.connector();
    }
    
    ValueConnector<SongEntry> method songEntryConnector()
    {
        return _songPlayed.connector();
    }
    
    ValueConnector<shared_ptr<IPlaylist>> method playlistCurrentlyPlayingConnector()
	{
    	return _playlistCurrentlyPlaying.connector();
	}

    ValueConnector<bool> method playingConnector()
    {
        return _playing.connector();
    }
    
    shared_ptr<PromisedImage> method albumArt(const long size) const
    {
        SongEntry entry = _songPlayed;
        return IApp::instance()->albumArtStash()->art(entry.song(), size);
    }
    
    SongGrouping method groupingCurrentlyPlaying() const
    {
        return _groupingCurrentlyPlaying;
    }
    
    ManagedValue<bool> & method shuffle()
    {
        return _shuffle;
    }
    
    ManagedValue<IPlayer::Repeat> & method repeat()
    {
        return _repeat;
    }

    ManagedValue<int> & method rating()
    {
        return _rating;
    }
    
    shared_ptr<IPlaylist> method playlistCurrentlyPlaying() const
    {
        return _playlistCurrentlyPlaying;
    }

    EventConnector method ratedConnector()
    {
    	return _ratedEvent.connector();
    }
}
