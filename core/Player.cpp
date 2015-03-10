//
//  Player.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "Player.h"
#include "PlaybackController.h"
#include "stdplus.h"
#include "PlaybackData.h"
#include "BackgroundExecutor.h"
#include "MainExecutor.h"
#include "Environment.h"
#include "LastFmService.h"
#include "IApp.h"
#include "Bridge.h"
#include "sessionManager.h"
#include "IPlaylist.h"
#include "SongData.h"

namespace Gear
{
#define method Player::
    
    shared_ptr<Player> method create()
    {
        shared_ptr<Player> ret(new Player());
        ret->init();
  #ifndef _WIN32
        ret->initRemote();
  #endif
        return ret;
    }
    
    method Player()
    {
    }

    void method initRemote()
    {
        auto bridge = IApp::instance()->bridge();
        auto self = shared_from_this();
        bridge->installPost("playToggle", [self](const Json::Value &){
            self->play();
        }, Bridge::Privilege::RemoteControl);
        bridge->installPost("playNext", [self](const Json::Value &){
            self->next();
        }, Bridge::Privilege::RemoteControl);
        bridge->installPost("playPrevious", [self](const Json::Value &){
            self->prev();
        }, Bridge::Privilege::RemoteControl);
        bridge->installPost("rateTogglePlaying", [self](const Json::Value &){
            self->rate();
        }, Bridge::Privilege::RemoteControl);
        bridge->installPost("playSetRatio", [self](const Json::Value &arg){
            self->setRatio(arg.asFloat());
        }, Bridge::Privilege::RemoteControl);
        bridge->installPost("play", [self,this](const Json::Value &args){

            auto playlistId = args.get("playlistId","").asString();
            auto categories = IApp::instance()->sessionManager()->categories();
            shared_ptr<IPlaylist> playlist;
            for (const auto &category : *categories) {
                for (const auto &playlistCandidate : category.playlists()) {
                    if (playlistCandidate->playlistId() == playlistId) {
                        playlist = playlistCandidate;
                        break;
                    }
                }  
                if (playlist) {
                    break;
                }
            }
            if (!playlist) {
                return;
            }

            SongEntry song(SongData(args.get("song",Json::objectValue)));

            auto forceRestart = args.get("forceRestart",0).asBool();

            PlayerBase::play(playlist, song, forceRestart);
        }, Bridge::Privilege::RemoteControl);
        bridge->installSubscribe("playingSong", [self,this](const Json::Value &arg, const function<void(const Json::Value &)> &callback){
            auto c = _songPlayed.connector().connect([callback](const SongEntry &entry){
                callback(entry.toJson());
            });
            _remoteConnections.push_back(c);
        }, [](const Json::Value &arg){
            // unsubscribe handler
        }, Bridge::Privilege::RemoteControlParty);
        bridge->installSubscribe("playingState", [self,this](const Json::Value &arg, const function<void(const Json::Value &)> &callback){
            auto send = [self,this,callback]{
                Json::Value value;
                value["playing"] = _playing.get();
                value["ratio"] = _songRatio.get();
                value["remainingTime"] = _remainingTime.get();
                value["elapsedTime"] = _elapsedTime.get();
                callback(value);
            };

            auto c = _playing.connector().connect([send](bool){send();});
            _remoteConnections.push_back(c);
            c = _songRatio.connector().connect([send](float){send();});
            _remoteConnections.push_back(c);
            c = _elapsedTime.connector().connect([send](string){send();});
            _remoteConnections.push_back(c);
            c = _remainingTime.connector().connect([send](string){send();});
            _remoteConnections.push_back(c);

        }, [](const Json::Value &arg){
            // unsubscribe handler
        }, Bridge::Privilege::RemoteControlParty);
        bridge->installSubscribe("playingRating", [self,this](const Json::Value &arg, const function<void(const Json::Value &)> &callback){
            auto c = _rating.connector().connect([callback](int rating){
                callback(rating);
            });
            _remoteConnections.push_back(c);

        }, [](const Json::Value &arg){
        }, Bridge::Privilege::RemoteControlParty);
        bridge->installSubscribe("playingMode", [self,this](const Json::Value &arg, const function<void(const Json::Value &)> &callback){
            auto send = [self,this,callback]{
                Json::Value value;
                value["shuffle"] = _shuffle.get();
                value["repeat"] = static_cast<int>(_repeat.get());
                callback(value);
            };

            auto c = _shuffle.connector().connect([send](bool){send();});
            _remoteConnections.push_back(c);
            c = _repeat.connector().connect([send](Repeat){send();});
            _remoteConnections.push_back(c);
        }, [](const Json::Value &arg){
        }, Bridge::Privilege::RemoteControlParty);
    }

    void method play()
    {
        if (_playing) {
            PlaybackController::instance()->pauseSong();
            _playing = false;
        } else {
            auto song = PlaybackController::instance()->songPlayed();
            if (song) {
                PlaybackController::instance()->playSong(song);
                _playing = true;
            }
        }
    }
    
    bool method restartLastSong()
    {
        return PlaybackController::instance()->restartLastSong();
    }
    
    void method doSetRatio(float ratio)
    {
        PlaybackController::instance()->setRatio(ratio);
    }
    
    void method playTimePassed(float duration)
    {
        LastFmService::playTimePassed(duration);
    }
    
    bool method currentFinishedDownloading()
    {
        return PlaybackController::instance()->currentFinishedDownloading();
    }
    
    void method cacheSong(const shared_ptr<ISong> &song, shared_ptr<IPlaybackData> data)
    {
        PlaybackController::instance()->cacheSong(song, data);
    }
}
