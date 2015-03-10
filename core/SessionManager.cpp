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
#include "Color.h"
#include "GroupedPlaylist.h"
#include "YouTubeSession.h"
#include "YouTubeService.h"
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
#include "IoService.h"
#include "sfl/Vector.h"
#include "sfl/Maybe.h"
#include "sfl/Map.h"
#include "Predicate.h"
#include "Db.h"
#include "BasicPlaylist.h"
#include "SortedSongArray.h"

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
    
    method SessionManager(const shared_ptr<IApp> &app) :
        //ISession(app),
        _offline(false),
        _categoriesTransformer(IApp::instance()->phoneInterface() ? new PhoneCategoriesTransformer() : nullptr)
#if TARGET_OS_IPHONE || !defined(DISABLE_OFFLINE_ON_DESKTOP)
//        ,_offlineSession(OfflineSession::create(app))
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
            //auto result = session->searchSync(filter, t);
            //copy(result.begin(), result.end(), back_inserter(ret));
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
        return playlistFrom(ClientDb::Predicate("playlist","all"), "all", "My Library",nullptr);
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
        assert(Io::isCurrent());

        using namespace sfl;        

        auto remoteControl = IApp::instance()->remoteControl();
        if (remoteControl && remoteControl->controlling()) {
            auto result = IApp::instance()->remoteControl()->playlistCategories();
            if (_categoriesTransformer) {
                result = _categoriesTransformer->transform(result);
            }     
            return shared_ptr<const std::vector<PlaylistCategory>>(new std::vector<PlaylistCategory>(result));
        }

        shared_ptr<const vector<shared_ptr<ISession>>> sessions = _sessions;
        
        auto sessionIdentifiers = sfl::map([](const shared_ptr<ISession> &s){return s->sessionIdentifier();}, *sessions);
        auto sessionPredicates = sfl::map([](const string &identifier){return ClientDb::Predicate("source",identifier);}, sessionIdentifiers);
        auto playlistData = IApp::instance()->db()->fetchSync(
                    "Playlist", 
                    ClientDb::Predicate(ClientDb::Predicate::Operator::Or,sessionPredicates),
                    ClientDb::SortDescriptor({"category","name"},true),
                    0,0,false);
        const vector<Json::Value> presences = sfl::map([](const shared_ptr<ISession> &s){
            auto dbSession = MEMORY_NS::dynamic_pointer_cast<DbSession>(s);
            return dbSession ? dbSession->behaviour.presence() : Json::Value();
        }, *sessions);
        auto presentWithName = [&](const std::string &identifier, const std::string &defaultName)->Maybe<string>{
            std::vector<string> names = sfl::bind([&](const Json::Value &presence){
                return presence.isMember(identifier) 
                        ? Vector::singleton(presence.get(identifier,"").asString()) 
                        : Vector::empty<string>();
            }, presences);
            if (names.size() == 0) {
                return Nothing();
            } else if (names.size() == 1) {
                return names.at(0);
            } else {
                return defaultName;
            }
        };
        auto libraryPresent = presentWithName("all", "My Library");
        auto searchPresent = presentWithName("free", "Search");

        auto libraryFiltered = [](const SongPredicate &predicate, const string &name){
            return playlistFrom(ClientDb::Predicate::compositeAnd(ClientDb::Predicate("playlist","all"),(ClientDb::Predicate)predicate),
                                predicate.fingerPrint(), 
                                name, 
                                nullptr); 
        };

        std::map<string,vector<shared_ptr<IPlaylist>>> categoriesToPlaylists = [&]{
            const auto onCategory = [](const Json::Value &lhs,const Json::Value &rhs){return lhs["category"] == rhs["category"];};
            const vector<std::pair<string,vector<shared_ptr<IPlaylist>>>> grouped =
                    sfl::map([&](const vector<Json::Value> &group){
                        return std::make_pair(head(group)["category"].asString(), 
                                              sfl::map([&](const Json::Value &playlist)->shared_ptr<IPlaylist>{
                                                    return playlistFromData(playlist);
                                                    },group));
                    },groupByR(onCategory,playlistData));
            return Map::fromAscRange(grouped);
        }();
        auto categoryFromStoredPlaylists = [&](const string &identifier, const string &defaultName, int tag){
            Maybe<string> present = presentWithName(identifier, defaultName);
            return isJust(present) 
                    ? PlaylistCategory(maybe("",present), tag, false, categoriesToPlaylists[identifier])
                    : PlaylistCategory();
        };

        auto noPlaylist = Vector::empty<shared_ptr<IPlaylist>>();
#pragma message("TODO: auto playlists as before")
#pragma message("TODO: queue")
#pragma message("TODO: highly rated only if 4star enabled")
        auto categories = snoc(Vector::empty<PlaylistCategory>(),
                PlaylistCategory("All", kAllTag, false,
                    (isJust(searchPresent) ? Vector::singleton(playlistFrom(ClientDb::Predicate("playlist","temp:free"), "free", maybe("", searchPresent),nullptr,{std::make_pair("freeSearch","1")})) : noPlaylist) +
                    (isJust(libraryPresent) ? Vector::singleton(playlistFrom(ClientDb::Predicate("playlist","all"), "all", maybe("", libraryPresent),nullptr)) : noPlaylist)
                ),
                categoryFromStoredPlaylists("playlist", "Playlists", kPlaylistsTag),
                categoryFromStoredPlaylists("radio", "Radio", kRadioTag),
                categoryFromStoredPlaylists("subscription", "Subscriptions", kSubscriptionsTag),
                PlaylistCategory("Auto", kAutoPlaylistsTag, false, 
                            {libraryFiltered(SongPredicate("rating",5), "Thumbs Up"),
                             libraryFiltered(SongPredicate("rating", 4, SongPredicate::GreaterOrEquals()), "Highly Rated"),
                             libraryFiltered(SongPredicate("rating",1), "Thumbs Down"),
                             libraryFiltered(SongPredicate("rating", 0), "Unrated"),
                             libraryFiltered(SongPredicate(SongPredicate::recentlyAdded(IApp::instance()->preferences())), "Recently Added"),
                             QueuePlaylist::create(),
                             libraryFiltered(SongPredicate("offlineRatio", 0, SongPredicate::GreaterThan()), "Saved For Offline")
                         }
                        //Vector::singleton((shared_ptr<IPlaylist>)_unions.thumbsUpPlaylist) +
                        //(IApp::instance()->preferences().boolForKey("FiveStarRatings") ? Vector::singleton(_unions.highlyRatedPlaylist) : noPlaylist) 
                        //+ Vector::singleton((shared_ptr<IPlaylist>)_unions.thumbsDownPlaylist)
                )
                ); 

        return shared_ptr<const vector<PlaylistCategory>>(new vector<PlaylistCategory>(std::move(
            filter([](const PlaylistCategory &category){return (bool)category;},categories))));
    }

    void method search(const std::string &filter)
    {
        shared_ptr<const vector<shared_ptr<ISession>>> sessions = _sessions;
        for (auto session : *sessions) {
            session->search(filter);
        }
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
    	if (session) {
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
		if (session) {
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
        
        Io::get().dispatch([current,this,self]{
            
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


    EventConnector method playlistsChangeEvent()
    {
        return _playlistsChangeSignal.connector();
    }

    PlaylistCategory method categoryByTag(int tag)
    {
        auto cat = categories();
        for (auto it = cat->begin() ; it != cat->end() ; ++it) {
            auto &category = *it;
            if (category.tag() == tag) {
                return category;
            }
        }
        static PlaylistCategory notFound;
        return notFound;
    }

    ValueConnector<bool> method refreshingConnector()
    {
        return _refreshing.connector();
    }
}
