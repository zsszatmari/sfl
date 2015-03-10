#include "PlayerRemote.h"
#include "RemoteControl.h"
#include "json.h"
#include "IApp.h"
#include "IPlaylist.h"
#include "SongData.h"

namespace Gear
{
#define method PlayerRemote::

    shared_ptr<PlayerRemote> method create(const shared_ptr<RemoteControl> &control)
    {
        shared_ptr<PlayerRemote> ret(new PlayerRemote(control));
        ret->initRemote();

        return ret;
    }

    void method initRemote()
    {
        auto self = shared_from_this();
        auto control = _control.lock();
        if (!control) {
            return;
        }
        control->subscribe("playingSong", Json::Value(), [self,this](const Json::Value &value){
            _songPlayed = SongEntry(SongData(value));
        });

        control->subscribe("playingState", Json::Value(), [self,this](const Json::Value &value){
            _playing = value.get("playing",false).asBool();
            _songRatio = value.get("ratio",0).asFloat();
            _elapsedTime = value.get("elapsedTime", "00:00").asString();
            _remainingTime = value.get("remainingTime", "00:00").asString();
        });

        control->subscribe("playingRating", Json::Value(), [self,this](const Json::Value &value){
            _rating = value.asInt();
        });

        control->subscribe("playingMode", Json::Value(), [self,this](const Json::Value &value){
            _shuffle = value.get("shuffle",false).asBool();
            _repeat = static_cast<Repeat>(value.get("repeat",0).asInt());
        });
        
        // are these two needed in some form?
        //SongGrouping _groupingCurrentlyPlaying;
        //ManagedValue<shared_ptr<IPlaylist>> _playlistCurrentlyPlaying;
    }

	method PlayerRemote(const shared_ptr<RemoteControl> &control) :
		_control(control)
	{
	}
	
	void method play(const SongEntry &song)
	{
        play(IApp::instance()->selectedPlaylistConnector().value().second, song);
	}

    void method play()
    {
    	auto control = _control.lock();
    	if (control) {
    		control->post("playToggle",Json::Value());
    	}
    }

    void method play(const shared_ptr<IPlaylist> &playlist, const SongEntry &song, bool forceRestart)
    {
        if (!playlist) {
            return;
        }

        Json::Value args(Json::objectValue);
        args["playlistId"] = playlist->playlistId();
        args["song"] = song.toJson();
        args["forceRestart"] = forceRestart;
        IApp::instance()->remoteControl()->post("play", args);
    }

    void method next()
    {
        IApp::instance()->remoteControl()->post("playNext", Json::Value());
    }

    void method prev()
    {
        IApp::instance()->remoteControl()->post("playPrevious", Json::Value());
    }

    void method setRatio(float ratio)
    {
        IApp::instance()->remoteControl()->post("playSetRatio", Json::Value(ratio));
    }

    void method rate(const shared_ptr<ISong> &song)
    {
    }

    void method rate(const shared_ptr<ISong> &song, int selected)
    {
    }

    void method rate()
    {
        IApp::instance()->remoteControl()->post("rateTogglePlaying", Json::Value());
    }
}