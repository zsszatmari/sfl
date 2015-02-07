#include "PlayerSwitch.h"
#include "MainExecutor.h"

namespace Gear
{
#define method PlayerSwitch::

	method PlayerSwitch(const shared_ptr<IPlayer> _player)
	{
		_players.push_back(_player);
        refreshConnections();
	}	

	void method push(const shared_ptr<IPlayer> _player)
	{
		MainExecutor::instance().addTask([=]{
			_players.push_back(_player);
            refreshConnections();
		});
	}

	void method pop()
	{
		MainExecutor::instance().addTask([=]{
			while (_players.size() > 1) {
                _players.pop_back();
                refreshConnections();
			}
		});
	}

	void method play(const SongEntry &song)
	{
		current()->play(song);	
	}

    void method prev()
    {
    	current()->prev();
    }

    void method play()
    {
    	current()->play();
    }

    void method next()
    {
    	current()->next();
    }

    void method rate(const shared_ptr<ISong> &song)
    {
    	current()->rate(song);
    }

    void method rate(const shared_ptr<ISong> &song, int selected)
    {
    	current()->rate(song, selected);
    }

    void method rate()
    {
    	current()->rate();
    }
    
    void method setRatio(float ratio)
    {
    	current()->setRatio(ratio);
    }

    shared_ptr<IPlayer> method current() const
    {
    	return _players.back();
    }

    void method refreshConnections()
    {
        _conns.clear();

        _conns.push_back(current()->songRatioConnector().connect([=](float value){
            _songRatio = value;
        }));
        _conns.push_back(current()->elapsedTimeConnector().connect([=](string value){
            _elapsedTime = value;
        }));
        _conns.push_back(current()->remainingTimeConnector().connect([=](string value){
            _remainingTime = value;
        }));
        _conns.push_back(current()->songEntryConnector().connect([=](const SongEntry &value){
            _songPlayed = value;
        }));
        _conns.push_back(current()->playingConnector().connect([=](bool value){
            _playing = value;
        }));
        _conns.push_back(current()->ratedConnector().connect([=](){
            _ratedEvent.signal();
        }));
        _conns.push_back(current()->playlistCurrentlyPlayingConnector().connect([=](const shared_ptr<IPlaylist> value){
            _playlistCurrentlyPlaying = value;
        }));

        _conns.push_back(current()->shuffle().connector().connect([=](bool value){
            _shuffle = value;
        }));
        _conns.push_back(current()->repeat().connector().connect([=](IPlayer::Repeat value){
            _repeat = value;
        }));
        // it's the other direction
        _conns.push_back(_rating.connector().connect([=](int value){
            current()->rating() = value;
        }));
    }

    shared_ptr<PromisedImage> method albumArt(const long imageSize) const
    {
        return current()->albumArt(imageSize);
    }

    SongGrouping method groupingCurrentlyPlaying() const
    {
        return current()->groupingCurrentlyPlaying();
    }
}