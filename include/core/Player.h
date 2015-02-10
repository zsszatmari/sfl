//
//  Player.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__Player__
#define __G_Ear_iOS__Player__

#include "PlayerBase.h"

namespace Gear
{
    class ISong;
    
    class Player final : public PlayerBase
    {
    public:
        static shared_ptr<Player> create();
        void songFinished(shared_ptr<ISong> &songPlayed);
        virtual void play();
        
    private:
        Player();
        Player(const Player &rhs); // delete
        Player &operator=(const Player &rhs); // delete

        void initRemote();
        std::vector<SignalConnection> _remoteConnections;
        
        virtual bool restartLastSong();
        virtual void doSetRatio(float ratio);
        virtual void playTimePassed(float duration);
        virtual bool currentFinishedDownloading();
        virtual void cacheSong(const shared_ptr<ISong> &song, shared_ptr<IPlaybackData> data);
    };
}

#endif /* defined(__G_Ear_iOS__Player__) */
