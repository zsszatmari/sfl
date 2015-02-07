//
//  SessionManager.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/6/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <iostream>
#include "SessionManager.h"
#include "GooglePlaySession.h"
#include "UnionPlaylist.h"
#include "Color.h"
#include "GroupedPlaylist.h"
#include "YouTubeSession.h"
#include "YouTubeService.h"
#include "PredicatePlaylist.h"
#include "NamedImage.h"
#include "IApp.h"
#include "Logger.h"
#include "OfflineSession.h"
#include "OfflineIntent.h"
#include "PhoneCategoriesTransformer.h"
#include "QueuePlaylist.h"
#include "IPreferences.h"
#include "Bridge.h"
#include "BaseUtility.h"
#include "RemoteControl.h"
#include "sfl/vector.h"

namespace Gear
{
#define method SessionManager::
    
    using std::cout;
    using std::endl;
    
    shared_ptr<SessionManager> method create(const shared_ptr<IApp> &app)
    {
        auto self = shared_ptr<SessionManager>(new SessionManager(app));
     
        if (self->_offlineSession) {
            self->addSession(self->_offlineSession);
        } else {
            // compute anyway
            self->recomputeCategories();
        }
        self->_fiveStarConnection = IApp::instance()->preferenceConnect("FiveStarRatings", [self]{
            self->recomputeCategories();
        });
        self->_fiveStarConnection = IApp::instance()->preferenceConnect("ShowAlbumArtistsTab", [self]{
            self->recomputeCategories();
        });
        
        auto bridge = IApp::instance()->bridge();
        bridge->installGet("categories", [self](const Json::Value &)->Json::Value{
            return PlaylistCategory::serialize(*self->_categories.get());
        }, Bridge::Privilege::RemoteControlParty);

        return self;
    }
    
    SessionManager::Unions method createUnions()
    {
        PlaylistCategory all(kAllCategory, kAllTag, true);
        all.setSingularPlaylist(false);
        PlaylistCategory autoPlay(kAutoPlaylistsCategory, kAutoPlaylistsTag, false);
        
        //#ifndef DISABLE_YOUTUBE
        shared_ptr<UnionPlaylist> freePlaylist(new UnionPlaylist("free", "Search"));
        //all.playlists().push_back(free);
        //#endif
        shared_ptr<UnionPlaylist> libraryPlaylist(new UnionPlaylist("all", "My Library"));
        all.playlists().push_back(libraryPlaylist);
        shared_ptr<UnionPlaylist> thumbsUpPlaylist(new UnionPlaylist(PredicatePlaylist::thumbsUpPredicate().fingerPrint(), "Thumbs Up"));
        autoPlay.playlists().push_back(thumbsUpPlaylist);
        autoPlay.playlists().push_back(PredicatePlaylist::thumbsDownPlaylist(libraryPlaylist->unionArray()));
        autoPlay.playlists().push_back(PredicatePlaylist::unratedPlaylist(libraryPlaylist->unionArray()));
        autoPlay.playlists().push_back(PredicatePlaylist::recentlyAddedPlaylist(libraryPlaylist->unionArray()));

        //shared_ptr<UnionPlaylist> thumbsDownPlaylist(new UnionPlaylist(PredicatePlaylist::thumbsDownPredicate().fingerPrint(), "Thumbs Down"));
        //autoPlay.playlists().push_back(thumbsDownPlaylist);

        shared_ptr<QueuePlaylist> queuePlaylist(QueuePlaylist::create());
        autoPlay.playlists().push_back(queuePlaylist);
        
        Unions unions;
        
        auto &unionArray = libraryPlaylist->unionArray();
        if (!IApp::instance()->phoneInterface()) {
            // these would take unacceptable amount of memory
            all.playlists().push_back(GroupedPlaylist::artistsPlaylist(unionArray));
            all.playlists().push_back(GroupedPlaylist::albumsPlaylist(unionArray));
            all.playlists().push_back(GroupedPlaylist::genresPlaylist(unionArray));
            all.playlists().push_back(GroupedPlaylist::albumArtistsPlaylist(unionArray));
        } else {
            unions.artistsForPhone = GroupedPlaylist::artistsPlaylist(unionArray);
            all.playlists().push_back(unions.artistsForPhone);
            // see albumArtistsForPhone
            //all.playlists().push_back(GroupedPlaylist::albumArtistsPlaylist(unionArray));
        }
        
        PlaylistCategory c[] = {all, autoPlay};
        pair<string, shared_ptr<UnionPlaylist>> u[] =
        {std::make_pair("all", libraryPlaylist),
            std::make_pair("free", freePlaylist),
            std::make_pair(thumbsUpPlaylist->playlistId(), thumbsUpPlaylist)
        //    ,std::make_pair(thumbsDownPlaylist->playlistId(), thumbsDownPlaylist)
        };
        
        unions.thumbsUpPlaylist = thumbsUpPlaylist;
        unions.categories = init<vector<PlaylistCategory>>(c);
        unions.playlists = init<UNORDERED_NS::unordered_map<string, shared_ptr<UnionPlaylist>>>(u);
        unions.libraryPlaylist = libraryPlaylist;
        unions.highlyRatedPlaylist = PredicatePlaylist::highlyRatedPlaylist(libraryPlaylist->unionArray(), thumbsUpPlaylist->unionArray());
        
        return unions;
    }
    
    method SessionManager(const shared_ptr<IApp> &app) :
        ISession(app),
        _offline(false),
        _unions(createUnions()),
        _categoriesTransformer(IApp::instance()->phoneInterface() ? new PhoneCategoriesTransformer() : nullptr)
#if TARGET_OS_IPHONE || !defined(DISABLE_OFFLINE_ON_DESKTOP)
        ,_offlineSession(OfflineSession::create(app))
#endif
    {
    }
    
    const vector<SongEntry> method searchSync(const string &filter, string &token)
    {
        // used by script call. only fetch first results
        vector<SongEntry> ret;
        shared_ptr<const vector<shared_ptr<ISession>>> sessions = _sessions;
        for (auto &session : *sessions) {
            string t;
            auto result = session->searchSync(filter, t);
            copy(result.begin(), result.end(), back_inserter(ret));
        }
        return ret;
    }
    
    shared_ptr<IPlaybackData> method playbackDataSync(const ISong &song) const
    {
        return shared_ptr<IPlaybackData>();
        // TODO
    }
    
    void method addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
    {
        // TODO
    }
    
    void method onConnected()
    {
        _connectedSignal.signal();
    }
    
    void method onPlaylistChange()
    {
        recomputeCategories();
    }
    
    shared_ptr<IPlaylist> method libraryPlaylist()
    {
        return _unions.libraryPlaylist;
    }
    
    const shared_ptr<IPlaylist> method freeSearchPlaylist()
    {
    	auto cat = categories();
    	for (auto it = cat->begin() ; it != cat->end() ; ++it) {
    		auto &category = *it;
    		auto &playlists = category.playlists();
    		for (auto itPlaylist = playlists.begin() ; itPlaylist != playlists.end() ; ++itPlaylist) {
    			auto &playlist = *itPlaylist;
    			if (playlist->playlistId() == "free") {
    				return playlist;
    			}
    		}
        }
        return libraryPlaylist();
    }
    
    /*static PlaylistCategory &findOrCreateCategory(vector<PlaylistCategory> &result, const PlaylistCategory &similarTo)
    {
        auto it = find_if(result.begin(), result.end(), [&](const PlaylistCategory &rhs){
            return rhs.tag() == similarTo.tag();
        });
        
        if (it == result.end()) {
            result.push_back(similarTo);
            
        }
    }*/
    
    ValidPtr<const vector<PlaylistCategory>> method categories() const
    {
        return _categories;
    }
    
    ValidPtr<const vector<PlaylistCategory>> method doRecomputeCategories()
    {
        using std::map;

        auto remoteControl = IApp::instance()->remoteControl();
        if (remoteControl && remoteControl->controlling()) {
            auto result = IApp::instance()->remoteControl()->playlistCategories();
            if (_categoriesTransformer) {
                result = _categoriesTransformer->transform(result);
            }     
            return shared_ptr<const std::vector<PlaylistCategory>>(new std::vector<PlaylistCategory>(result));
        }

        ValidPtr<vector<PlaylistCategory>> pResult = ValidPtr<vector<PlaylistCategory>>(new vector<PlaylistCategory>(_unions.categories));
        auto &result = *pResult;
        result.erase(result.begin() +1, result.end());
        
        map<shared_ptr<UnionPlaylist>, vector<shared_ptr<IPlaylist>>> unionElements;
        
        // we must have something even if we have no sessions!
        for (auto it = _unions.playlists.begin() ; it != _unions.playlists.end() ; ++it) {
        	auto &unionList = *it;
        	unionElements[unionList.second];
        }
    
        vector<shared_ptr<ISession>> sessions = _sessions;
#if DEBUG
        std::cout << "counting for " << sessions.size() << " sessions\n";
        for (auto session : sessions) {
            using namespace sfl;
            auto mcategory = maybe(PlaylistCategory(),maybeAt(*session->categories(), 0));
            auto name = [&](int index){
                return match<std::string>(maybeAt(mcategory.playlists(),index), 
                                     [](Nothing){return "nada";},
                                     [](const shared_ptr<IPlaylist> &playlist){return playlist->playlistId();});
            };
            std::cout << "session: " << session->sessionIdentifier() 
                      << " first category: " << mcategory.title()
                      << " first/second playlist: " << name(0) << "/" << name(1) << std::endl;
        }
#endif
    
        if (_offline) {
            auto it = remove_if(sessions.begin(), sessions.end(), [&](const shared_ptr<ISession> &session){
                return session != _offlineSession;
            });
            sessions.erase(it, sessions.end());
        }
        
        bool youtubePresent = false;
        int otherPresent = 0;
    	for (auto it = sessions.begin() ; it != sessions.end() ; ++it) {
    		if (MEMORY_NS::dynamic_pointer_cast<YouTubeSessionBase>(*it)) {
    			youtubePresent = true;
    		} else {
    			otherPresent++;
    		}
    	}
        
        // since there is offline mode we shall always display the icon to avoid strangeness
        _needSessionIcons = true;
    	//_needSessionIcons = (youtubePresent && otherPresent) || (otherPresent >= 2);

        //int i = 0;
        for (auto itSession = sessions.begin() ; itSession != sessions.end() ; ++itSession) {
            
            //cout << "session:" << i++ << endl;
            
        	const auto &session = *itSession;

            auto sessionCategories = session->categories();
            for (auto itCategory = sessionCategories->begin() ; itCategory != sessionCategories->end() ; ++itCategory) {
            	const auto &sessionCategory = *itCategory;

                auto it = find_if(result.begin(), result.end(), [&](const PlaylistCategory &rhs){
                    return rhs.tag() == sessionCategory.tag();
                });
                
                if (it == result.end()) {
                    auto it2 = find_if(_unions.categories.begin(), _unions.categories.end(), [&](const PlaylistCategory &rhs){
                        return rhs.tag() == sessionCategory.tag();
                    });
                    if (it2 != _unions.categories.end()) {
                        result.push_back(*it2);
                        it = result.end() -1;
                    }
                }
                
                if (it == result.end()) {
                    // add new category if not already there. we have nothing to do with them
                    result.push_back(sessionCategory);
                } else {
                    PlaylistCategory &resultCategory = *it;
                    auto &playlistsInSessionCategory = sessionCategory.playlists();
                    for (auto it = playlistsInSessionCategory.begin() ; it != playlistsInSessionCategory.end() ; ++it) {
                        
                    	const auto &playlistInSessionCategory = *it;
                    	if (!playlistInSessionCategory) {
                    		continue;
                    	}
                    	auto playlistId = playlistInSessionCategory->playlistId();
                        
                        //cout << "playlistid:" << playlistId << endl;
                        
                        auto itUnion = _unions.playlists.find(playlistId);
                        auto &playlistsInResultCategory = resultCategory.playlists();
                        if (itUnion == _unions.playlists.end()) {
                            //cout << "not found union " << playlistId << endl;
                            playlistsInResultCategory.push_back(playlistInSessionCategory);
                        } else {
                            //cout << "found union " << playlistId << endl;
                            const shared_ptr<UnionPlaylist> unionPlaylist = itUnion->second;
                            unionElements[unionPlaylist].push_back(playlistInSessionCategory);
                            
                            // add _union_ if not yet added

                            /*if (playlistId == "free") {
                                cout << "already:\n";
                                for (auto &playlist : playlists) {
                                    cout << playlist->playlistId() << endl;
                                }
                            }*/
                            auto itAdd = find(playlistsInResultCategory.begin(), playlistsInResultCategory.end(), unionPlaylist);
                            if (itAdd == playlistsInResultCategory.end()) {
                                /*cout << "notfound: " << unionPlaylist->name() << "/" << unionPlaylist->playlistId() << endl;
                                for (auto &already : playlistsInResultCategory) {
                                    cout << "already: " << already->name() << "/" << already->playlistId() << "/" (playlistInSessionCategory == ) endl;
                                }*/
                                playlistsInResultCategory.insert(playlistsInResultCategory.begin(), unionPlaylist);
                            } else {
                                //cout << "found\n";
                            }
                        }
                    }
                }
            }
        }
        
        auto libraryPlaylistTouched = !IApp::instance()->phoneInterface();
        for (auto it = unionElements.begin() ; it != unionElements.end() ; ++it) {
            auto &playlist = it->first;
            
            playlist->setPlaylists(it->second);
#if DEBUG
            std::cout << "playlist " << playlist->playlistId() << " " << it->second.size() << std::endl;
#endif
            if (it->second.size() == 0) {
                if (IApp::instance()->phoneInterface()) {
                    // remove 'auto' or library union playlists if empty
                    for (auto it = result.begin() ; it != result.end() ; ++it) {
                        if (it->tag() == kAutoPlaylistsTag) {
                            auto &playlists = it->playlists();
                            playlists.erase(remove(playlists.begin(), playlists.end(), playlist), playlists.end());
                            break;
                        }
                    }
                }
            } else {
                
                if (playlist == _unions.libraryPlaylist) {
                    libraryPlaylistTouched = true;
                }
            }
        }
        
        for (auto it = result.begin() ; it != result.end() ; ++it) {
            if (it->tag() == kAutoPlaylistsTag) {
                if (it->playlists().empty()) {
                    result.erase(it);
                } else {
                    if (IApp::instance()->preferences().boolForKey("FiveStarRatings")) {
                        PlaylistCategory &category = *it;
                        vector<shared_ptr<IPlaylist>> &autoPlaylists = category.playlists();
                        autoPlaylists.insert(autoPlaylists.begin() +1, _unions.highlyRatedPlaylist);
                    }
                }
                break;
            }
            if (IApp::instance()->phoneInterface() && it->tag() == kAllTag && IApp::instance()->preferences().boolForKey("ShowAlbumArtistsTab")) {
                auto &playlists = it->playlists();
                auto itArtists = find(playlists.begin(), playlists.end(), _unions.artistsForPhone);
                if (itArtists != playlists.end()) {
                    if (!_albumArtistsForPhone) {
                        _albumArtistsForPhone = GroupedPlaylist::albumArtistsPlaylist(_unions.libraryPlaylist->unionArray());
                    }
                    playlists.insert(itArtists+1, _albumArtistsForPhone);
                }
            }
        }
        
        if (!libraryPlaylistTouched) {
        	// clear library on logout
#if DEBUG
            std::cout << "library null" << std::endl;
#endif
        	_unions.libraryPlaylist->setPlaylists(vector<shared_ptr<IPlaylist>>());
        }
        
        if (_categoriesTransformer) {
            result = _categoriesTransformer->transform(result);
        }

        if (!libraryPlaylistTouched) {
            auto it = remove_if(result.begin(), result.end(), [](const PlaylistCategory &c){
                return c.tag() == kAllTag || c.tag() == kArtistsTag || c.tag() == kAlbumArtistsTag;
            });
            result.erase(it, result.end());
        }
        
        
      /*  std::cout << "categories:\n";
        for (auto &cat : *pResult) {
            std::cout << cat.title() << std::endl;
            if (cat.tag() == kAutoPlaylistsTag) {
                std::cout << "playlists: \n";
                for (auto playlist : cat.playlists()) {
                    std::cout << " " << playlist->name() << std::endl;
                }
            }
        }*/

        return pResult;
    }
    
    void method refresh()
    {
        auto sessions = _sessions;
        
    	for (auto it = sessions->begin() ; it != sessions->end() ; ++it) {
    		auto &session = *it;
    		session->refresh();
        }
    }
    
    void method onRefreshingChange(bool aValue)
    {
        vector<shared_ptr<ISession>> sessions = _sessions;
        
        bool newValue = false;
        for (auto it = sessions.begin() ; it != sessions.end() ; ++it) {
        	const auto &session = *it;
        	if (session->refreshingConnector().value()) {
                newValue = true;
                break;
            }
        }
        
        _refreshing = newValue;
    }
    
    void method addSession(const shared_ptr<ISession> &session)
    {
        _sessions.set([&](vector<shared_ptr<ISession>> &sessions){
            sessions.push_back(session);
        });
        
        session->setOffline(_offline);
        
        session->connectedEvent().connect(&SessionManager::onConnected, shared_from_this());
        session->playlistsChangeEvent().connect(&SessionManager::onPlaylistChange, shared_from_this());
        session->refreshingConnector().connect(&SessionManager::onRefreshingChange, shared_from_this());
        
        recomputeCategories();
        // filter criteria might have changed because of session change, especially at my library
        // must call reselectCurrentPlaylist() somewhere... but here is probably not the best point
        // since sessions might add library later
    }
    
    void method removeSession(const shared_ptr<ISession> &session)
    {
        _sessions.set([&](vector<shared_ptr<ISession>> &sessions){
            sessions.erase(remove(sessions.begin(), sessions.end(), session), sessions.end());
        });
        if (session) {
            session->dispose();
        }
        
        recomputeCategories();
    }

    void method reselectCurrentPlaylist()
    {
        _recomputeExecutor.addTask([]{

            auto oldValue = IApp::instance()->selectedPlaylistConnector().value();
#ifdef DEBUG
            std::cout << "reselecting playlist " << (oldValue.second ? oldValue.second->playlistId() : "nada") << "TODO: also do this with e.g. thumbs up!!!\n";
#endif

            if (oldValue.second && oldValue.second->playlistId() == "all") {
                IApp::instance()->doSetSelectedPlaylist(oldValue, true);
            }
        });
    }

    shared_ptr<ISession> method session(const string &identifier) const
    {
        shared_ptr<const vector<shared_ptr<ISession>>> sessions = _sessions;
        for (auto &session : *sessions) {
            if (session->sessionIdentifier() == identifier) {
                return session;
            }
        }
        return shared_ptr<ISession>();
    }
    
    
    shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<ISong> &song) const
	{
    	// forget about singularness, just show the icon, for consistency's sake
    	//bool singular = IApp::instance()->selectedPlaylistCategoryConnector().value().singularPlaylist();
    	if (!song) {
            return nullptr;
        }
        auto session = song->session();
    	if (_needSessionIcons /*&& singular*/ && session) {
    		return session->sessionIcon(song);
    	}
    	return shared_ptr<Gui::IPaintable>();
	}

    shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<IPlaylist> &playlist) const
	{
        if (!playlist) {
            return nullptr;
        }
    	auto session = playlist->session();
		if (_needSessionIcons && session) {
			return session->sessionIcon(playlist);
		}
		return shared_ptr<Gui::IPaintable>();
	}
    
    void method freeUpMemory()
    {
        vector<shared_ptr<ISession>> sessions = _sessions;
        for (auto &session : sessions) {
            session->freeUpMemory();
        }
    }
    
    void method recomputeCategories()
    {
        // this might pose a problem when we come back, let it be
        /*if (IApp::instance()->remoteControl()->controlling()) {
            return;
        }*/
        _recomputeState = shared_ptr<bool>(new bool(true));
        weak_ptr<bool> current = _recomputeState;
        
        //std::cout << "recomputing should begin" << current.lock().get() << "\n";
        auto self = shared_from_this();
        
        _recomputeExecutor.addTask([current,this,self]{
            
            if (current.expired()) {
                return;
            }
            //std::cout << "recomputing begin " << current.lock().get() << "\n";
            
            auto result = doRecomputeCategories();
            
            /*
            if (current.expired()) {
                return;
            }*/
            
            //std::cout << "recomputing end " << result.get()->size() << "\n";
            
            _categories = result.get();
            _playlistsChangeSignal.signal();

            reselectCurrentPlaylist();
        });
    }

    void method setOffline(bool offline)
    {
        _offline = offline;
        auto sessions = _sessions;
        for (auto &session : *sessions.ptr()) {
            session->setOffline(offline);
        }
        
        recomputeCategories();
    }
    
    std::string method sessionIdentifier() const
    {
        return "manager";
    }
    
    vector<int> method possibleRatings() const
    {
        return vector<int>();
    }
}
