//
//  IApp.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/5/13.
//
//

#include "IApp.h"
#include "ISession.h"
#include "AlbumArtStash.h"
#include "IPlayer.h"
#include "SessionManager.h"
#include "IPlaylist.h"
#include "CastPlaybackWorker.h"
#include "QueueSongArray.h"
#include "SongSortOrder.h"
#include "Timer.h"
#include "IPreferences.h"
#include "Bridge.h"
#include "IoService.h"
#include "Equalizer.h"

namespace Gear
{
#define method IApp::
    
    using THREAD_NS::lock_guard;
    using THREAD_NS::mutex;

    method IApp() :
        _userExplicitlySelected(false),
        _bridge(new Bridge())
    {
    }
    
    method ~IApp()
    {
    }
    
    ValueConnector<std::pair<PlaylistCategory,shared_ptr<IPlaylist>>> method selectedPlaylistConnector()
    {
        return __selectedPlaylist.connector();
    }
    
    static shared_ptr<IApp> _instance;
    
    shared_ptr<IApp> method instance()
    {
        return _instance;
    }
    
    void method registerInstance(const shared_ptr<IApp> &app)
    {
        _instance = app;
    }
    
    shared_ptr<AlbumArtStash> method albumArtStash()
    {
        if (!_albumArtStash) {
            _albumArtStash = shared_ptr<AlbumArtStash>(new AlbumArtStash(imageCacheDirectory(), fileManager()));
        }
        
        return _albumArtStash;
    }
    
    shared_ptr<IPlaylist> method selectedPlaylist() const
    {
        return __selectedPlaylist.get().second;
    }
    
    bool method userSelectedCategory(int tag, const shared_ptr<IPlaylist> &aPlaylist, bool start, bool implicit)
    {
        if (!implicit) {
            _userExplicitlySelected = true;
        }
        shared_ptr<IPlaylist> playlist = aPlaylist;
        auto category = sessionManager()->categoryByTag(tag);
        
        if (category.tag() == tag) {
            
            if (!playlist && category.singularPlaylist()) {
                shared_ptr<IPlaylist> singularPlaylist = category.playlists()[0];
                playlist = singularPlaylist;
            } 
            
            doSetSelectedPlaylist(std::make_pair(category, playlist));
            
            if (!playlist) {
                return false;
            }
            
            if (start) {
                playFirstOfCurrentPlaylist();
            }
            return true;
        }
        return false;
    }
    
    void method userSelectedPlaylist(const shared_ptr<IPlaylist> &playlist, bool start)
    {
        std::pair<PlaylistCategory,shared_ptr<IPlaylist>> selection;
        if (playlist) {
            selection = playlist->selected();
        }
        if (!selection.first) {
            selection = std::make_pair(__selectedPlaylist.get().first,playlist);
        }
        doSetSelectedPlaylist(selection);
        if (start) {
            if (playlist && playlist->songArray() == QueueSongArray::instance()) {
                if (!IApp::instance()->player()->playingConnector().value()) {
                    IApp::instance()->player()->play();
                }
            } else {
                playFirstOfCurrentPlaylist();
            }
        }
    }
    
    void method goBack()
    {
        PlaylistCategory category = IApp::instance()->selectedPlaylistConnector().value().first;
        if (category.level() > 0) {
            
            auto tabCategory = sessionManager()->categoryByTag(category.tag());
            if (tabCategory) {
                doSetSelectedPlaylist(std::make_pair(tabCategory, nullptr));
            }
        }
    }
    
    shared_ptr<LocalSession> method createLocalSession()
    {
        return shared_ptr<LocalSession>();
    }

    bool method trialMode() const
    {
        return false;
    }
    
    shared_ptr<IPlaybackWorker> method playbackWorker()
    {
        return shared_ptr<IPlaybackWorker>(CastPlaybackWorker::create());
    }

    void method terminate()
    {
        playbackWorker()->terminate();
        lock_guard<mutex> l(_terminateMutex);
        for (auto &c : _terminateCallbacks) {
            c();
        }

        Io::terminate();
    }

    void method addTerminateCallback(const function<void()> &f)
    {
        lock_guard<mutex> l(_terminateMutex);
        _terminateCallbacks.push_back(f);
    }

    void method disableAppNap()
    {
    }
    
    void method enableAppNap()
    {
    }

    shared_ptr<ISongArray> method queueSongArray() const
    {
        return _queueSongArray;
    }

    void method recalculateQueue()
    {
        QueueSongArray::instance()->recalculate();
    }

    void method doSetSelectedPlaylist(const std::pair<PlaylistCategory,shared_ptr<IPlaylist>> &p, bool force)
    {
        if (p.second) {
            SortDescriptor descriptor;
            if (p.second->keepSongOrder()) {
                descriptor = IPlaylist::sortDescriptorForId(p.second->playlistId());
            } else {
                descriptor = SongSortOrder::sortDescriptor();
            }
            p.second->setSortDescriptor(descriptor);
        }
        if (force || __selectedPlaylist.get() != p) {
            __selectedPlaylist = p;
        }
    }

    SignalConnection method preferenceConnect(const string &key, const function<void()> &f)
    {
        shared_ptr<SignalConnectionToken> conn(new SignalConnectionToken(f));
        weak_ptr<SignalConnectionToken> wconn = conn;
        shared_ptr<bool> lastValue(new bool(preferences().boolForKey(key)));
        
        new Base::Timer([wconn,key,lastValue](Timer *timer){
            auto sconn = wconn.lock();
            if (sconn) {
                bool newValue = IApp::instance()->preferences().boolForKey(key);
                if (newValue != *lastValue) {
                    *lastValue = newValue;
                    sconn->signal();
                }
            } else {
                delete timer;
            }
        }, 1.0f, true, 0.5f);

        return conn;
    }

    shared_ptr<LastFmController> method lastFmController() const
    {
        return _lastFmController;
    }

    shared_ptr<Bridge> method bridge() const
    {
        return _bridge;
    }

    shared_ptr<Db> method db() const
    {
        return _db;
    }

    void method showRemoteConnectPanel()
    {
    }


    shared_ptr<IEqualizer> method equalizer()
    {
        if (!_equalizer) {
            _equalizer = shared_ptr<IEqualizer>(new Equalizer());
        }
        return _equalizer;
    }

    void method applyTheme()
    {
    }
}
