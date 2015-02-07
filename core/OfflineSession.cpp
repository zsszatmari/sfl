//
//  OfflineSession.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 01/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "OfflineSession.h"
#include "PlaylistCategory.h"
#include "OfflineSongArray.h"
#include "PredicatePlaylist.h"
#include "OfflineStorage.h"
#include "NamedImage.h"
#include "OfflineService.h"

namespace Gear
{
#define method OfflineSession::
    
    shared_ptr<OfflineSession> method create(const shared_ptr<IApp> &app)
    {
        auto ret = shared_ptr<OfflineSession>(new OfflineSession(app));
        ret->createCategories();
        return ret;
    }
    
    vector<int> method possibleRatings() const
    {
        vector<int> ret;
    	ret.push_back(0);
    	return ret;
    }
    
    method OfflineSession(const shared_ptr<IApp> &app) :
        ISession(app),
        _offline(false)
    {
        _updateCounter = 0;
    }
    
    void method createCategories()
    {
        {
            vector<PlaylistCategory> categories;
        
            PlaylistCategory autoPlaylists = PlaylistCategory(u(kAutoPlaylistsCategory), kAutoPlaylistsTag);
            
            _savedForOfflineArray = shared_ptr<OfflineSongArray>(OfflineSongArray::create(shared_from_this()));
            
            _libraryPlaylist = shared_ptr<PredicatePlaylist>(new PredicatePlaylist(_savedForOfflineArray, "My Library", SongPredicate()));
            _libraryPlaylist->setPlaylistId("all");
            _libraryPlaylist->setSaveForOfflinePossible(false);
            
            _savedForOfflinePlaylist = shared_ptr<PredicatePlaylist>(new PredicatePlaylist(_savedForOfflineArray, "Saved For Offline", SongPredicate()));
            _savedForOfflinePlaylist->setPlaylistId("savedforoffline");
            _savedForOfflinePlaylist->setSaveForOfflinePossible(false);
       
        
            autoPlaylists.playlists().push_back(_savedForOfflinePlaylist);
            categories.push_back(autoPlaylists);
            
            _categories = categories;
        }
        {
            vector<PlaylistCategory> fullCategories;
            
            PlaylistCategory all = PlaylistCategory(kAllCategory, kAllTag, true);
            all.setSingularPlaylist(false);
            all.playlists().push_back(_libraryPlaylist);
            fullCategories.push_back(all);
            
            PlaylistCategory playlistsCategory(kPlaylistsCategory, kPlaylistsTag, false);
            fullCategories.push_back(playlistsCategory);
            //_fullCategories.push_back(autoPlaylists);
            
            _fullCategories = fullCategories;
        }
    
        // assumption: OfflineSession is living forever
        auto update = [this]{
            
            _savedForOfflineArray->refresh();
            
            ++_updateCounter;
            _executor.addTask([this]{
                
                if (_updateCounter == 1) {
					shared_ptr<OfflineSession> pThis = this->shared_from_this();
					auto gotPlaylists = OfflineStorage::instance().playlists(pThis);
                
                    _fullCategories.setS([this,gotPlaylists](shared_ptr<vector<PlaylistCategory>> &fullCategories){
                        auto it = fullCategories->begin() ;
                        for ( ; it != fullCategories->end() ; ++it) {
                            auto &category = *it;
                            if (category.tag() == kPlaylistsTag) {
                                
                                auto &playlists = category.playlists();
                                playlists = gotPlaylists;
                                
                                _playlistsChangeSignal.signal();
                                
                                break;
                            }
                        }
                        return fullCategories;
                    });
                }
                
                --_updateCounter;
            });
        };
        
        
        _updateConnection = OfflineStorage::instance().updatedEvent().connect(update);
        update();
    }
    
    shared_ptr<IPlaylist> method libraryPlaylist()
    {
        return _libraryPlaylist;
    }
    
    void method refresh()
    {
    }
    
    void method addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
    {
    }
    
    shared_ptr<IPlaybackData> method playbackDataSync(const ISong &song) const
    {
        const string uniqueId = song.uniqueId();
        if (OfflineStorage::instance().available(uniqueId)) {
            static const string youtube("youtube");
            return OfflineStorage::instance().fetch(uniqueId, song.stringForKey("sourceService") == youtube ? IPlaybackData::Format::Mp4 : IPlaybackData::Format::Mp3);
        }
        return nullptr;
    }
    
    ValidPtr<const vector<PlaylistCategory>> method categories() const
    {
        if (!OfflineService::available() || OfflineService::disabledStatic()) {
            return ValidPtr<const vector<PlaylistCategory>>();
        }
        
        //return _fullCategories;
        if (_offline) {
            return _fullCategories;
        } else {
            return _categories;
        }
    }
    
    static std::string sessionIconName(const shared_ptr<ISong> &song)
    {
        std::string session = song->stringForKey(OfflineStorage::SourceSessionKey);
        if (!session.empty()) {
            return "favicon-" + session;
        }
        return "";
    }
    
    shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<ISong> &song) const
    {
        return shared_ptr<Gui::IPaintable>(new Gui::NamedImage(sessionIconName(song)));
    }
    
    shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<IPlaylist> &playlist) const
    {
        string ret;
        #pragma message("TODO: can't reliably get session icon")
        /*auto songsPtr = playlist->songArray()->rawSongs();
        for (auto &song : *songsPtr) {
            auto candidate = sessionIconName(song.song());
            if (candidate != ret) {
                if (ret.empty()) {
                    ret = candidate;
                } else {
                    ret = "";
                    break;
                }
            }
        }
        if (ret.empty()) {
            return shared_ptr<Gui::IPaintable>();
        }
        return shared_ptr<Gui::IPaintable>(new Gui::NamedImage(ret));*/
        return shared_ptr<Gui::IPaintable>();
    }
    
    void method setOffline(bool offline)
    {
        _offline = offline;
    }
    
    std::string method sessionIdentifier() const
    {
        return "offline";
    }
    
    bool method saveForOfflinePossible() const
    {
        // actually, this is for remove from offline
        return true;
    }
}