    //
//  PlaybackController.m
//  googlemusic2
//
//  Created by Zsolt Szatmári on 3/7/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <ctime>
#include <sstream>
#include "PlaybackController.h"
#include "PlaybackItem.h"
#include "IApp.h"
#include "IPlaybackWorker.h"
#include "MainExecutor.h"
#include "PlaybackControllerDelegate.h"
#include "Environment.h"
#include <iostream>
#include "LastFmService.h"
#include "GearUtility.h"
#include "IPreferences.h"
//#import "Debug.h"

namespace Gear
{
#define method PlaybackController::

    using THREAD_NS::lock_guard;
    using THREAD_NS::this_thread::sleep_for;
    using THREAD_NS::recursive_mutex;
    
    shared_ptr<PlaybackController> method create()
    {
        shared_ptr<PlaybackController> ret(new PlaybackController());
        ret->_playbackWorker->setController(ret);
        ret->_timer.addTask([ret]{
            while (true) {
                CHRONO_NS::milliseconds w(250);
                sleep_for(w);
                ret->updateTime();
            }
        });
        
        return ret;
    }
    
    shared_ptr<PlaybackController> method instance()
    {
        static shared_ptr<PlaybackController> singleton = PlaybackController::create();
        return singleton;
    }
    
    method PlaybackController() :
        _alreadyToldAboutFormat(false),
        _cachedDuration(0),
        _wasPlaying(false),
        _playbackWorker(IApp::instance()->playbackWorker())
    {
    }
    
    int method getUncompressedDataInto(char *dataPtr, int length, uint8_t)
    {
        // direct access, we must be fast here
        shared_ptr<PlaybackItem> audio = _audio;
        if (!audio) {
            return 0;
        }
        
        int read = 0;
        if (dataPtr != nullptr) {
            //std::cout << "song finished: " << audio->isSongFinished() << std::endl;
            read = audio->getUncompressedDataInto(dataPtr, length);
            
            //std::cout << "read data: " << read << std::endl;
            if (read < length && audio->isSongFinished()) {
                
                songFinished();
                
                // seamlessly continue to next song!
                
                // continuing:
                int addedLength = getUncompressedDataInto(dataPtr + read, length - read, 0);
                return read + addedLength;
                
                // or overwrite audio data?
                /*if (cachedAudio != nil) {
                 return [self getUncompressedDataInto:dataPtr length:length];
                 }*/
                
                // or maybe we should mix them together?
            }
        }
        
        // this not necessary, as we work with constant 32-bit floats
        //if (!alreadyToldAboutFormat && _audio.gotStreamDescription) {
        //    [playbackWorker setAudioFormat:_audio.streamDescription];
        //}
        
        return read;
    }
    
    shared_ptr<PlaybackItem> method audio()
    {
        return _audio;
    }
    
    void method setAudio(shared_ptr<PlaybackItem> value)
    {
        _alreadyToldAboutFormat = false;
        _audio = value;
    }

    void method updatePlayCount(const shared_ptr<ISong> &song)
    {
        if (!song) {
            return;
        }
        
        LastFmService::songBegan(song);

        song->setPlayCount(song->playCount()+1);
        song->setLastPlayed(((long long)std::time(NULL)) * 1000 * 1000);
        song->save();
    }    
    
    void method songFinished()
    {
        //abort();

#ifdef DEBUG_NONEXTSONG
        NSLog(@"songfinished %d", cachedAudio != nil);
#endif
        
        // this will change in the process...
        auto oldSongPlayed = _songPlayed;
        
        _restartAudio = audio();
        _restartSong = _songPlayed;
        
        
        
        if (_cachedAudio) {
            
            setAudio(_cachedAudio);
            _cachedAudio.reset();
            _songPlayed = _cachedSong;
            _cachedSong.reset();
            
            updatePlayCount(_songPlayed);
            // go on...
        } else {
            _playbackWorker->stop();
            setAudio(shared_ptr<PlaybackItem>());
        }
        
        auto delegate = _delegate;
        Base::MainExecutor::instance().addTask([delegate, oldSongPlayed]{
        
            // nevertheless, playback should continue without this!
            // comment this out for testing purposes only
            if (delegate) {
                delegate->songFinished(oldSongPlayed);
            }
        });
    }
    
    void method updateTime()
    {
    	auto au = audio();
        if (au) {
            
            float current = au->currentTime();
            //NSAssert([NSThread isMainThread], @"song time update should be in main thread");
            
            // we rely on metadata for the total
            //NSTimeInterval total = self.audio.duration;
            //NSTimeInterval displayTotal = total;
            float displayTotal = 0;
            if (displayTotal == 0) {
                displayTotal = total();
            }
            
            if (_delegate) {
                _delegate->setElapsedTime(current);
                // don't lose 1 because of rounding...
                int time = (int)displayTotal-(int)current;
                if (time < 0) {
                	time = 0;
                }
                _delegate->setRemainingTime(time);
                _delegate->changedRatio(displayTotal > 0 ? current/displayTotal : 0);
            }
        }
    }
    
    
    float method total()
    {
        if (!equals(_songPlayed, _cachedDurationForSongPlayed)) {
            if (_songPlayed) {
                _cachedDuration = ((float)_songPlayed->durationMillis()) / 1000.0f;
            } else {
                _cachedDuration = 0;
            }
            _cachedDurationForSongPlayed = _songPlayed;
        }
        
        return _cachedDuration;
    }
    
    void method pauseSong()
    {
        _wasPlaying = false;
        _playbackWorker->stop();
        _pausedSong = _songPlayed;
    }
    
    shared_ptr<ISong> method songPlayed()
    {
        return _songPlayed;
    }

    bool method playSong(shared_ptr<ISong> song)
    {
        lock_guard<recursive_mutex> l(_songMutex);
        
        _restartAudio.reset();

        if (song && equals(_songPlayed, song)) {
            _playbackWorker->play();
        } else if (song && equals(_cachedSong, song)) {
            // [song songObject]: song or playlist entry we don't care
            
            //NSLog(@"cache hit!");
            
            setAudio(_cachedAudio);
            _cachedAudio.reset();
            _songPlayed = _cachedSong;
            _cachedSong.reset();
            
            _playbackWorker->play();
            
            updatePlayCount(song);
            
        } else if (!song) {
            
            _delegate->setElapsedTime(0);
			_delegate->setRemainingTime(0);
			_delegate->changedRatio(0);
            
            _wasPlaying = false;
            _playbackWorker->stop();
            setData(shared_ptr<PlaybackData>());
            _songPlayed.reset();
            updateTime();
        } else {

			_playbackWorker->stop(false);
			_songPlayed = song;
			setAudio(shared_ptr<PlaybackItem>());

			_delegate->setElapsedTime(0);
			_delegate->setRemainingTime(total());
			_delegate->changedRatio(0);

			//updateTime();
            bool shouldSkip = [&]{
                std::istringstream is(IApp::instance()->preferences().stringForKey("BlackList"));
                string token;
                auto artist = song->artist();
                while (std::getline(is,token,',')) {
                    if (!token.empty() && token == artist) {
                        return true;
                    } 
                }
                return false;
            }();
            shared_ptr<IPlaybackData> data;
            if (!shouldSkip) {
                data = song->playbackDataSync();
            }

            if (data) {
                setData(data);
                
                // this was a wrong solution,
                /*if (_pausedSong.lock() != _songPlayed) {
                    _playbackWorker->play();
                } else {
#ifdef DEBUG
                    std::cout << "dontplay: notpaused\n";
#endif
                }*/
                _playbackWorker->play();
                
                updateTime();
                updatePlayCount(song);
                
            } else {
#ifdef DEBUG
                std::cout << "dontplay: no data\n";
#endif
            	
                setData(data);
            	songFinished();
                _playbackWorker->stop(true);
                return false;
            }
        }
        return true;
    }
    
    void method setData(shared_ptr<Gear::IPlaybackData> data)
    {
        lock_guard<recursive_mutex> l(_songMutex);
        if (data) {
            setAudio(audioWithData(data));
        } else {
            setAudio(shared_ptr<PlaybackItem>());
        }
    }
    
    shared_ptr<PlaybackItem> method audioWithData(shared_ptr<Gear::IPlaybackData> data)
    {
        shared_ptr<PlaybackItem> item = PlaybackItem::create(data);
        item->setDelegate(shared_from_this());
        
        return item;
    }
    
    void method rateChangedForPlaybackItem(const PlaybackItem &aItem, float rate)
    {
        auto pThis = shared_from_this();
        Base::MainExecutor::instance().addTask([pThis, &aItem, rate]{
            // see valse bizarre to test this
            if (&aItem == pThis->audio().get()) {
                bool isPlaying = rate != 0;
                if (pThis->_wasPlaying && !isPlaying) {
                    pThis->_delegate->songFinished(pThis->_songPlayed);
                    pThis->_playbackWorker->stop();
                }
                pThis->_wasPlaying = isPlaying;
            }
        });
    }

    void method setDelegate(shared_ptr<PlaybackControllerDelegate> delegate)
    {
        _delegate = delegate;
    }
    
    bool method restartLastSong()
    {
        bool restart = false;

        // this if caused problems! (when deleting cachedaudio..)
        //if (!audio()) {
            setAudio(_restartAudio);
            restart = true;
        //}
        if (!audio()) {
        	return false;
        }

        // rewind
        setRatio(0);
        
        if (restart) {
            _songPlayed = _restartSong;
        }
        //restartAudio = nil;
        //restartAudioObjectId = nil;
        _playbackWorker->play();
        
        updatePlayCount(_songPlayed);
        
        return true;
    }

    void method setRatio(float ratio)
    {
        auto a = audio();
        if (a) {
            float tot = total();
            a->setCurrentTime(ratio * tot, ratio);
            updateTime();
        }
    }
    
    bool method currentFinishedDownloading()
    {
        auto a = audio();
        if (a) {
            return a->finishedDownloading();
        } else {
            return false;
        }
    }
    
    void method cacheSong(shared_ptr<ISong> song, shared_ptr<IPlaybackData> aData)
    {
        if (aData) {
            _cachedAudio = audioWithData(aData);
            _cachedSong = song;
        }
    }
    
#if TARGET_OS_MACDESKTOP
    bool method resetVolume()
    {
        return _playbackWorker->resetVolume();
    }
#endif
    
    void method setVolume(float volume)
    {
        _playbackWorker->setVolume(volume);
    }
    
    void method setRateIfPlaying(float rate)
    {
        auto a = audio();
        if (a) {
            a->setRateIfPlaying(rate);
        }
    }
}

