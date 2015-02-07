//
//  AppBase.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <iostream>
#include "AppBase.h"
#include "SessionManager.h"
#ifndef NON_APPLE
#include "AppleKeychain.h"
#endif
#include "Player.h"
#include "ServiceManager.h"
#include "ThemeManager.h"
#include "ISongArray.h"
#include "MainExecutor.h"
#include "PreferencesPanel.h"
#include "SongPredicate.h"
#include "stdplus.h"
#include "SongSortOrder.h"
#include "Logger.h"
#include "Chain.h"
#include "ValidPtr.h"
#include "JavascriptEngineTiny.h"
#include "IPlaylist.h"
#include "IAudioDeviceEnumerator.h"
#include "Tracking.h"
#include "QueueSongArray.h"
#include "IPlaybackWorker.h"
#include "PlaybackController.h"
#include "LastFmController.h"
#include "HttpDownloader.h"
#include "RemoteControlServer.h"
#include "RemoteControl.h"
#include "PlayerSwitch.h"
#include "Db.h"

#ifdef USE_BOOST_INSTEAD_OF_CPP11
#include "boost/random.hpp"
typedef boost::random::minstd_rand default_random_engine;
using boost::random::uniform_int_distribution;
#else
#include <random>
using std::default_random_engine;
using std::uniform_int_distribution;
#endif
#ifdef _WIN32
#ifdef BUILDING_CORE_DLL
// this causes problem
//#include <boost/chrono/detail/inlined/win/chrono.hpp>
#endif
#endif


namespace Gear
{
	using namespace Gui;

#define method AppBase::
    
    method AppBase()
    {
#ifdef TARGET_OS_IPHONE
        _keychain = shared_ptr<IKeychain>(new AppleKeychain());
#endif
        _themeManager = shared_ptr<ThemeManager>(new ThemeManager());
    }
    
    DragManager & method dragManager()
    {
        return _dragManager;
    }
    
    shared_ptr<IKeychain> method keychain()
    {
        return _keychain;
    }
    
    bool method phoneInterface()
    {
#if TARGET_OS_IPHONE || defined(TIZEN)
        return true;
#else
        return false;
#endif
    }
    
    void method appStarted()
    {
        /*bool canGo = false;
        while (!canGo) {
            std::cout << "waiting for debugger to connect. then set canGo to true (AppBase)\n";
            sleep(1);
        }*/

        _db = shared_ptr<Db>(new Db(dataPath() + "/" + "gear.db"));

        _player = shared_ptr<PlayerSwitch>(new PlayerSwitch(Player::create()));
        _queueSongArray = QueueSongArray::create(_player);
        
        _sessionManager = SessionManager::create(shared_from_this());
        _serviceManager = shared_ptr<ServiceManager>(new ServiceManager(_sessionManager));
        _preferencesPanel = shared_ptr<IPreferencesPanel>(new PreferencesPanel());
    
        shared_ptr<IPlaylist> selectedPlaylist = _sessionManager->libraryPlaylist();
        PlaylistCategory selectedPlaylistCategory;
        auto categories = _sessionManager->categories();
        for (auto it = categories->begin() ; it != categories->end() ; ++it) {
            if (it->defaultSelection()) {
                selectedPlaylistCategory = *it;
                break;
            }
        }
        
#ifdef DEBUGHOOK
        _selectedPlaylist.setDebugHook([]{
            std::cout << "set playlist\n";
        });
#endif
        
        doSetSelectedPlaylist(std::make_pair(selectedPlaylistCategory, selectedPlaylist));
        
#ifdef TARGET_OS_MACDESKTOP
        MainExecutor::instance().addTask([this]{
#endif
            // autoconnect may involve webview display, which causes problems on lion because cocoa is not initialized 
            // by the time appStarted is called
            _serviceManager->autoconnect();
#ifdef TARGET_OS_MACDESKTOP
        });
#endif
        
        IApp::instance()->sessionManager()->playlistsChangeEvent().connect(&AppBase::playlistsChanged,  MEMORY_NS::static_pointer_cast<AppBase>(shared_from_this()));
        playlistsChanged();

        Tracking::trackStart();

        playbackWorker()->setGenerator(PlaybackController::instance());

        _lastFmController = LastFmController::create();

	// init online verification (on mac it does not work correctly on the first time called because of an implementation detail)
        HttpDownloader::isOnline();

#if !TARGET_OS_IPHONE
        _rcServer = RemoteControlServer::create();
#else
        _rcClient = RemoteControl::create();
        // temporary hack to connect
        auto self = shared_from_this();

        /*class Hack : public RemoteControl::DiscoverDelegate
        {
        public:
            Hack(const shared_ptr<RemoteControl> rc) :
                _rc(rc)
            {   
            }

            virtual void foundRemoteControllable(const std::string &name, const string &ip, int port)
            {
                _rc->start(ip, port);
            }

        private:
            const shared_ptr<RemoteControl> _rc;
        };

        static shared_ptr<Hack> hack(new Hack(rcClient));

        rcClient->discover(hack);*/
#endif
    }

    shared_ptr<RemoteControl> method remoteControl() const
    {
        return _rcClient;
    }

    void method didBecomeActive()
    {
        _lastFmController->applicationDidBecomeActive();
    }
    
    SortDescriptor method defaultSongOrder() const
    {
        return SongSortOrder::sortDescriptor("");
    }
    
    void method playlistsChanged()
    {
        //std::cout << "playlistchanged " << _userExplicitlySelected << std::endl;
        
        if (IApp::instance()->phoneInterface()) {
            if (!_userExplicitlySelected) {
                //std::cout << "selecting LIB\n";
                if (!userSelectedCategory(kAllTag, nullptr, false, true)) {
                    //std::cout << "selecting SEARCH\n";
                    
                    userSelectedCategory(kFreeSearchTag, nullptr, false, true);
                }
                return;
            }
        }
        
        
        auto selected = selectedPlaylistConnector().value();
        if (selected.first.level() > 0) {
            return;
        }
        
        //bool found = false;
        auto categories = _sessionManager->categories();

        PlaylistCategory allCategory;
        for (auto it = categories->begin() ; it != categories->end() ; ++it) {
        	const auto &category = *it;
        	/*auto playlists = category.playlists();
            for (auto itPlaylist = playlists.begin() ; itPlaylist != playlists.end() ; ++itPlaylist) {
            	auto &playlist = *itPlaylist;

                if (playlist == selected) {
                    found = true;
                    break;
                }
            }*/
            
            if (((PlaylistCategory)selected.first).tag() == allCategory.tag()) {
                allCategory = selected.first;
            }

            if (((PlaylistCategory)selected.first).tag() == category.tag()) {
                selected.first = category;
                doSetSelectedPlaylist(selected);
                return;
            }

            // wrong:
            /*if (found) {
                break;
            }*/
        }
        
        //_selectedPlaylist = std::make_pair(allCategory, _sessionManager->libraryPlaylist());
    }
    
    shared_ptr<SessionManager> method sessionManager() const
    {
        return _sessionManager;
    }
    
    shared_ptr<ServiceManager> method serviceManager() const
    {
        return _serviceManager;
    }
    
    shared_ptr<ThemeManager> method themeManager() const
    {
        return _themeManager;
    }
    
    shared_ptr<IPreferencesPanel> method preferencesPanel() const
    {
        return _preferencesPanel;
    }
    
    shared_ptr<IPlayer> method player()
    {
        return _player;
    }

    shared_ptr<PlayerSwitch> method playerSwitch()
    {
        return _player;
    }
    
    void method playFirstOfCurrentPlaylist()
    {
        shared_ptr<IPlaylist> playlist = selectedPlaylist();
        auto array = playlist->songArray();

        #pragma message("TODO: play First Of CurrentPlaylist, check radios also on double click")

        weak_ptr<IPlayer> wPlayer = player();
        weak_ptr<ISongArray> wArray = array;
        static SongView songViewForPlaying;
        
        std::cout << "play first..." << std::endl;
        songViewForPlaying = array->songs([wPlayer,wArray](const SongView &songView,SongView::Event event,size_t offset,size_t size){
            
            std::cout << "play first:" << size << std::endl;
            if (event == SongView::Event::Inserted && size > 0) {

                auto player = wPlayer.lock();
                if (player) {
                    bool shuffle = player->shuffle();
                    SongEntry song;
                    if (!shuffle) {
                        song = songView.at(0);
                    } else {
                        static default_random_engine generator(CHRONO_NS::system_clock::now().time_since_epoch().count());
                        uniform_int_distribution<int> distribution(0,size-1);
                        
                        auto num = distribution(generator);
                        //std::cout << "starting shuffle " << num << std::endl;
                        song = songView.at(num);
                        auto array = wArray.lock();
                        if (array) {
                            array->setNoRepeatBase(song);
                        }
                    }
                    player->play(song);
                }
                songViewForPlaying = SongView();
            }
        });
        // trigger
        songViewForPlaying.size();
    }
    
    void method showRelated(const SongEntry &entry, const string &baseField)
    {
        auto playlist = _sessionManager->freeSearchPlaylist();
        //const auto &song = entry.song();
        string searchString;
      /*  if (baseField == "album") {
            searchString = song->stringForKey("artist") + " " + song->stringForKey("album");
        } else {
        */
        searchString = entry.song()->stringForKey(baseField);
        //}
        SongPredicate predicate("", u(searchString), SongPredicate::Contains());
        
        playlist->setFilterPredicate(predicate);

        
        userSelectedPlaylist(playlist);
        
    }
    
    void method freeUpMemory()
    {
        _sessionManager->freeUpMemory();
        db()->freeUpMemory();
    }
    
    shared_ptr<IJavascriptEngine> method createJavascriptEngine() const
    {
        return shared_ptr<IJavascriptEngine>(new JavascriptEngineTiny());
    }

    void method terminate()
    {
        _rcServer.reset();
        _rcClient.reset();

        IApp::terminate();
    }
};
