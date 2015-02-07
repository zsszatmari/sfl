//
//  PlayerBase.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/7/13.
//
//

#include <iostream>
#include "PlayerBase.h"
#include "IPlaylist.h"
#include "IApp.h"
#include "IPreferences.h"
#include "ISongArray.h"
#include "MainExecutor.h"
#include "BackgroundExecutor.h"
#include "stdplus.h"
#include "PlaybackController.h"
#include "Logger.h"
#include "ISession.h"
#include "HttpDownloader.h"
#include "Chain.h"
#include "ValidPtr.h"
#include "QueueSongArray.h"
#include "Tracking.h"
#include "serviceManager.h"
#include "PreferencesPanel.h"
#include "IoService.h"

namespace Gear
{
    using THREAD_NS::lock_guard;
    using THREAD_NS::this_thread::sleep_for;
    using THREAD_NS::mutex;

#define method PlayerBase::
    
    method PlayerBase() :
        _scheduling(false),
        _lastElapsedTime(0),
        _lastRemainingTime(0),
        _shuffleEnabledSinceSchedule(false),
        _delegateSet(false)
    {
        _lastNetworkFailure = 0;
        setElapsedTime(0);
        setRemainingTime(0);
    }

    void method play(const SongEntry &song)
    {
#ifdef DEBUG
//      std::cout << "play song " << song.entryId() << " / " << song.song()->uniqueId() << std::endl;
#endif
        play(IApp::instance()->selectedPlaylistConnector().value().second, song);

        /*
        auto categoryAndPlaylist = IApp::instance()->selectedPlaylistConnector().value();
        if (!categoryAndPlaylist.second) {
            return;
        }

        // this is not working yet, because (on osx) the category of the currently selected
        // playlist is not maintained well (it should be fixed before uncommenting this)
        
        if (categoryAndPlaylist.first.tag() == kAllTag) {
            Tracking::track("Playback", "Playlist Type", categoryAndPlaylist.second->name());
        } else {
            Tracking::track("Playback", "Playlist Type", categoryAndPlaylist.first.title());
        }*/
    }

    void method init()
    {
        IPreferences &prefs = IApp::instance()->preferences();
        _repeat = static_cast<IPlayer::Repeat>(prefs.uintForKey("repeat"));
        _shuffle = prefs.boolForKey("shuffle");
        
        _shuffle.connector().connect(&PlayerBase::shuffleChanged, shared_from_this());
        _repeat.connector().connect(&PlayerBase::repeatChanged, shared_from_this());
        _rating.connector().connect(&PlayerBase::ratingChanged, shared_from_this());
    }
    
    void method shuffleChanged(const bool change)
    {
        if (change) {
            _shuffleEnabledSinceSchedule = true;
        }
        auto queue = QueueSongArray::instance();
        if (queue) {
            queue->recalculate(_arrayForJumping, _songPlayed);
        }
        IApp::instance()->preferences().setUintForKey("shuffle", change);
    }
    
    void method repeatChanged(const IPlayer::Repeat change)
    {
        IApp::instance()->preferences().setUintForKey("repeat", static_cast<int>(change));
    }
    
    void method ratingChanged(const int change)
    {
    	SongEntry entry = _songPlayed;
    	shared_ptr<ISong> song = entry.song();
    	if (song) {

    		if (change != song->rating()) {
    			song->setRating(change);
				song->save();

				_ratedEvent.signal();
    		}
    	}
    }
    
    void method rate()
    {
        SongEntry entry = _songPlayed;
        rate(entry.song());
    }
    
    void method rate(const shared_ptr<ISong> &song)
    {
		vector<int> possibleRatings;
		if (song) {
			auto session = song->session();
			if (session) {
				possibleRatings = session->possibleRatings();
			}
		}

		if (possibleRatings.empty()) {
			return;
		}
		sort(possibleRatings.rbegin(), possibleRatings.rend());

		// 0 => 5 => 1 => 0
		possibleRatings.push_back(possibleRatings.at(0));
        
        auto currentRating = song->rating();
        
		auto it = find(possibleRatings.begin(), possibleRatings.end(), currentRating);
		if (it != possibleRatings.end()) {
			currentRating = *(it +1);
		} else if (!possibleRatings.empty()) {
            currentRating = possibleRatings.back();
        }
        
        if (currentRating != song->rating()) {
            song->setRating(currentRating);
            song->save();
        }
        
        SongEntry playedEntry = _songPlayed;
        if (song == playedEntry.song()) {
            _rating = currentRating;
        }

        _ratedEvent.signal();
    }

    void method rate(const shared_ptr<ISong> &song, int selected)
    {
        vector<int> possibleRatings;
        if (song) {
            auto session = song->session();
            if (session) {
                possibleRatings = session->possibleRatings();
            }
        }


        if (possibleRatings.empty()) {
            
            return;
        }

        auto oldRating = song->rating();


        if (selected == oldRating) {

            song->setRating(0);
        } else {
            auto it = find(possibleRatings.begin(), possibleRatings.end(), selected);
            if (it == possibleRatings.end()) {
                return;
            }
            song->setRating(selected);
        }
        song->save();


        SongEntry playedEntry = _songPlayed;
        if (song == playedEntry.song()) {
            _rating = song->rating();
        }
        _ratedEvent.signal();
    }

    void method play(const shared_ptr<IPlaylist> &playlist, const SongEntry &aSong, bool forceRestart)
    {
        auto self = shared_from_this();
        Io::get().dispatch([self,this,playlist,aSong,forceRestart]{
            // obviously nullptr is valid here 
            //assert(playlist);
            
            const auto song = aSong.unclone();

            if (playlist) {
                QueueSongArray::instance()->recalculate(playlist->songArray(), song);
                _arrayForJumping = playlist->songArray();
                _groupingCurrentlyPlaying = playlist->selectedGroupingConnector().value();
            } else {
                QueueSongArray::instance()->recalculate(nullptr, song);
                _arrayForJumping.reset();
                _groupingCurrentlyPlaying = SongGrouping();
            }


            bool playlistIsQueue = playlist && (playlist->songArray() == QueueSongArray::instance());
            
            if (song == _songPlayed) {
            	if (playlist == (shared_ptr<IPlaylist>)_playlistCurrentlyPlaying) {
            		if (!IApp::instance()->phoneInterface() || forceRestart) {
            			setRatio(0);
                        PlaybackController::updatePlayCount(song.song());
    			if (!_playing) {
                           	   play();
                        	}
            	    }
            	    return;
            	} else {
                    if (!playlistIsQueue) {
                		_playlistCurrentlyPlaying = playlist;
                    }
            		return;
            	}
            }
            // this must go first, so the mobile player screen will know where the song is from, and act accordingly
            if (!playlistIsQueue) {
                _playlistCurrentlyPlaying = playlist;
            }
            _songPlayed = song;
        
            if (song) {
                _rating = song.song()->rating();
                _playing = true;
                play(song.song());
                scheduleUpdateNextSong();
            } else {
                play(song.song());
            	_rating = 0;
                _playing = false;
            }
        });
    }
    
    static string stringFromTime(float timeFloat)
    {
        long time = timeFloat;
        long minutes = time / 60;
        long seconds = time % 60;
        
        char buf[64];
#ifdef _WIN32
#define snprintf _snprintf
#endif
        snprintf(buf, sizeof(buf), "%02ld:%02ld", minutes, seconds);
        return string(buf);
    }
    
    void method setElapsedTime(float elapsed)
    {
        _elapsedTime = stringFromTime(elapsed);
        
        float diff = elapsed - _lastElapsedTime;
        auto oldStamp = _lastElapsedTimeStamp;
        _lastElapsedTimeStamp = CHRONO_NS::steady_clock::now();
        
        auto realPassed = CHRONO_NS::duration_cast<CHRONO_NS::milliseconds>(_lastElapsedTimeStamp - oldStamp).count();
        float realTime = realPassed / 1000.0f;
        if (diff > 0.0f) {
            
            playTimePassed(realTime < diff ? realTime : diff);
        }
        _lastElapsedTime = elapsed;
    }
    
    void method setRemainingTime(float remaining)
    {
        _remainingTime = stringFromTime(remaining);
        _lastRemainingTime = remaining;
    }
    
    void method changedRatio(float ratio)
    {
        _songRatio = ratio;
    }
    
    void method clearSong()
    {
    	play(shared_ptr<IPlaylist>(), SongEntry());
		_songRatio = 0;
		setElapsedTime(0);
		setRemainingTime(0);
		_playing = false;
    }

    static uint64_t now()
    {
        using namespace CHRONO_NS;
        return duration_cast<milliseconds>(steady_clock::now() - steady_clock::time_point()).count();
    }

    void method songFinished()
    {
        SongEntry nextSong;
        
        // isOnline can be wrong.
	if (HttpDownloader::isOnline() || (now() - _lastNetworkFailure) > 10000) {
            
            nextSong = QueueSongArray::instance()->whatToPlayNextAfter(_songPlayed);
        }
        
#ifdef DEBUG
#define DEBUG_NONEXTSONG
#endif
#ifdef DEBUG_NONEXTSONG
        //std::cout << "finished: " << (std::string)_songPlayed.get().fastTitle() << " next: " << (std::string)nextSong.fastTitle() << std::endl;
#endif
        
        if (nextSong == _songPlayed) {
            //setRatio(0);
#ifdef DEBUG_NONEXTSONG
            std::cout << "debug1\n";
#endif
            if (!restartLastSong()) {
            	clearSong();
#ifdef DEBUG_NONEXTSONG
                std::cout << "debug2\n";
#endif
            }
            return;
        }
#ifdef DEBUG_NONEXTSONG
        std::cout << "debug3\n";
#endif
        
        if (nextSong) {
#ifdef DEBUG_NONEXTSONG
            std::cout << "debug4\n";
#endif
            play(_playlistCurrentlyPlaying, nextSong);
        } else {
            clearSong();
        }
    }
    
    void method setRatio(float ratio)
    {
        if (_lastElapsedTime == 0 && _lastRemainingTime == 0) {
            _songRatio = 0;
            return;
        }

        if (ratio < 0.0f) {
        	ratio = 0.0f;
        } else if (ratio > 1.0f) {
        	ratio = 1.0f;
        }

        doSetRatio(ratio);
    }
    
    void method prev()
    {
        Io::get().dispatch([this]{
            if (IApp::instance()->preferences().boolForKey("PressingBackRewindsSong")) {
                
                if (_lastElapsedTime > 2.0f) {
                    // on first click, go to beginning
                    setRatio(0);
                    return;
                }
            }

            next(false);
        });
    }

    void method next()
    {
        Io::get().dispatch([this]{
            if (!next(true)) {
                play(_playlistCurrentlyPlaying, SongEntry());
            }
            //songFinished();
        });
    }

    bool method next(bool forward)
    {
        SongEntry nextSong = QueueSongArray::instance()->whatToPlayNextAfter(_songPlayed, forward);
#ifdef DEBUG        
	std::cout << "player nextsong.. " << nextSong.song()->uniqueId() << "\n";
#endif
        if (nextSong) {
            play(_playlistCurrentlyPlaying, nextSong, true);
            return true;
        }
        return false;
    }

    bool method songValid(const SongEntry &entry) const
    {
        if (!entry) {
            return false;
        }
        return _arrayForJumping->contains(entry);
    }
    
    void method updateNextSong()
    {
        SongEntry songPlayed = _songPlayed;
        if (!songPlayed) {
            return;
        }
        if (_shuffle) {
            if (!_shuffleEnabledSinceSchedule && songValid(_cachedSong)) {
                return;
            } else {
                _cachedSong = QueueSongArray::instance()->whatToPlayNextAfter(_songPlayed).unclone();
            }
        } else {
            auto songToCache = QueueSongArray::instance()->whatToPlayNextAfter(_songPlayed).unclone();
            if (songToCache == _cachedSong) {
                return;
            } else {
                _cachedSong = songToCache;
            }
        }
        _shuffleEnabledSinceSchedule = false;
        auto retainedSong = _cachedSong.song();
        auto pThis = shared_from_this();
        
        if (_cachedSong) {
            BackgroundExecutor::instance().addTask([pThis, retainedSong]{
            
                auto data = retainedSong->playbackDataSync();
                
                MainExecutor::instance().addTask([pThis, retainedSong, data]{
                    
                    pThis->cacheSong(retainedSong, data);
                    //NSLog(@"caching next song..");
                    //[[PlaybackController sharedController] cacheSong:retainedSong withPlaybackData:unique_ptr<PlaybackData>(ptr)];
                });
            });
        }
    }
    
    void method scheduleUpdateNextSong()
    {
        if (_scheduling) {
            return;
        }
        _scheduling = true;
        
        shared_ptr<PlayerBase> self = shared_from_this();
        shared_ptr<boost::asio::deadline_timer> timer(new boost::asio::deadline_timer(Io::get(), 
            boost::posix_time::milliseconds(3000)));
        timer->async_wait([self,timer](const boost::system::error_code e){
                
            if (e) {
                return;
            }
            
            self->_scheduling = false;
            if (self->currentFinishedDownloading()) {
                self->updateNextSong();
            } else {
                self->scheduleUpdateNextSong();
            }
        });
    }
    
    void method songFinished(shared_ptr<ISong> songPlayed)
    {
        SongEntry entry = _songPlayed;
        if (songPlayed == entry.song()) {
            songFinished();
        }
    }
    
    void method play(const shared_ptr<ISong> &aSongToPlay)
    {
        if (!_delegateSet) {
            _delegateSet = true;
            
            PlaybackController::instance()->setDelegate(shared_from_this());
        }
        
        {
            lock_guard<mutex> l(_playInterruptMutex);
            if (_playInterruptHandle) {
                _playInterruptHandle->interrupt();
            }
        }
        
        shared_ptr<ISong> songToPlay = aSongToPlay;
        auto pThis = shared_from_this();
        _playExecutor.addTask([songToPlay, pThis]{
            
            Interruptor intp;
            {
                lock_guard<mutex> l(pThis->_playInterruptMutex);
                pThis->_playInterruptHandle = Interruptor::getHandle();
            }
            
            if (!pThis->_playing) {
                if (!songToPlay) {
                    PlaybackController::instance()->playSong(songToPlay);
                }
                return;
            }
            
			pThis->_lastPlayedSong = songToPlay;
			// hopefully we won't have problems for not executing this in the main queue... but playSong() must be
			// called as soon as possible!
			//MainExecutor::instance().addTask([songToPlay, playbackData, pThis]{

				//if (playbackData) {
                    if (!PlaybackController::instance()->playSong(songToPlay) && songToPlay) {
                        pThis->_lastNetworkFailure = now();
                    }
				/*} else {
					if (pThis->whatToPlayNextAfter(pThis->_songPlayed) == pThis->_songPlayed) {
						pThis->clearSong();
						PlaybackController::instance()->pauseSong();
						pThis->_playing = false;
					} else {
						PlaybackController::instance()->playSong(songToPlay, playbackData);
					}
				}*/
			//});

            if (songToPlay) {
                static int i = 0;
                Tracking::track("Playback", "Artist", songToPlay->artist());
                Tracking::track("Playback", "Genre", songToPlay->stringForKey("genre"));
                auto session = songToPlay->session();
                if (session) {
                    Tracking::track("Playback", "Source", session->sessionIdentifier());
                }
                if (i % 100 == 0) {
                    // only send this rarely
                    for (auto &service : IApp::instance()->serviceManager()->services()) {
                        auto available = PreferencesPanel::checkImmediately(*service);
                        if (!available) {
                            Tracking::track("Service", service->title(), "Gray");
                        } else {
                            Tracking::track("Service", service->title(), (service->stateConnector().value() == IService::State::Online) ? "On" : "Off");
                        }
                    }
                }
                ++i;
            }
        });
    }
}
