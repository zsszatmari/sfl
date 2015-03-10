#include <cassert>
#include <iostream>
#include "DbSession.h"
#include "IApp.h"
#include "Db.h"
#include "Tracking.h"
#include "sfl/Prelude.h"
#include "IoService.h"
#include "NamedImage.h"
#include "ISong.h"
#include "BasicPlaylist.h"
#include "IPlayer.h"

namespace Gear
{
#define method DbSession::

    using namespace sfl;

	method DbSession(const shared_ptr<IApp> &app, const Behaviour &aBehaviour) :
		SongManipulationSession(app),
		behaviour(aBehaviour)
	{
		behaviour.validate();
	}

	void DbSession::Behaviour::validate() const
	{
        assert(!sourceIdentifier.empty());
        assert(!name.empty());
        assert((bool)fetchLibrary);
        assert((bool)connect);
        assert((bool)presence);
        assert(!icon.empty());
        assert((bool)playback);
        assert((bool)fetchSearch);
        // there is no capability flag needed (it seems to be a bad idea, at least regarding all access), all is said in 'presence'
	}

    std::string method sessionIdentifier() const
    {
        return behaviour.sourceIdentifier;
    }

   	void method removeData()
   	{
   		IApp::instance()->db()->remove("Song", ClientDb::Predicate("source", behaviour.sourceIdentifier));
   		IApp::instance()->db()->remove("Playlist", ClientDb::Predicate("source", behaviour.sourceIdentifier));
   	}
    
    void method refresh()
    {
    	if (_refreshing) {
    		return;
    	}
    	_refreshing = true;
    	auto self = shared_from_this();
    	Io::get().dispatch([self,this]{
    		fetchLibrary();
    	});
    }

    void method connect(const string &user, const string &pass, const function<void(bool)> &callback)
    {
    	assert(Io::isCurrent());
    	_refreshing = true;

    	weak_ptr<DbSession> wself = shared_from_this();
    	behaviour.connect(user,pass,[callback,wself,this](bool success){
    		auto self = wself.lock();
    		if (!self) {
    			return;
    		}
    		if (success) {
                clearDynamic("");
				fetchLibrary();
				callback(true);
    		} else {
    			callback(false);
    		}
    	});
    }

    void method fetchLibrary()
    {
    	_toRemoveStartedCounting.clear();

    	weak_ptr<DbSession> wself = shared_from_this();
    	behaviour.fetchLibrary([wself](const string &entity,const vector<Json::Value> &items,bool finishedWithEntity){
			auto self = wself.lock();
			if (self) {
				self->gotData(entity, items, finishedWithEntity);
				return true;
			} else {
				return false;
			}
		});
    }

	void method gotData(const string &entity,const vector<Json::Value> &items, bool finishedWithEntity)
	{
    	assert(Io::isCurrent());
#ifdef DEBUG
        std::cout << "incoming data for " << behaviour.sourceIdentifier << ": " << entity << ": " << items.size() << " items " << (int)finishedWithEntity << std::endl; 
#endif

		auto removeKey = (entity == "Song") ? "entryId" : "playlistId";

		auto db = IApp::instance()->db();
		if (_toRemove[entity].empty() && !_toRemoveStartedCounting[entity]) {
			_toRemove[entity] = db->fetchSync(entity, ClientDb::Predicate("source",behaviour.sourceIdentifier), ClientDb::SortDescriptor({removeKey},true), removeKey);
			_toRemoveStartedCounting[entity] = true;
			if (entity == "Song") {
				_numberOfSongsBeforeFetch = _toRemove.size();
			}
		}

		for (auto item : items) {
			item["source"] = behaviour.sourceIdentifier;
			db->upsert(entity,item);
            /*if (entity == "Playlist") {
                Json::FastWriter writer;
                std::cout << "insetred " << writer.write(item) << std::endl; 
            }*/
		}

		vector<string> justAdded = sfl::map(
                    [&](const Json::Value &entry){return entry[removeKey].asString();}, items);
        sort(justAdded.begin(), justAdded.end());
        // The difference of two sets is formed by the elements that are present in the first set, but not in the second one. The elements copied by the function come always from the first range, in the same order.
        //std::cout << "todelete was: " << _toRemove[entity].size() << " incoming: " << justAdded.size() << std::endl;

        std::vector<std::string> diff;
        diff.reserve(_toRemove[entity].size());
        set_difference(_toRemove[entity].begin(), _toRemove[entity].end(), justAdded.begin(), justAdded.end(), back_inserter(diff));
        swap(_toRemove[entity], diff);
		//std::cout << "todelete new: " << _toRemove[entity].size() << " finished: " << (int)finishedWithEntity << std::endl;

		if (entity == "Playlist") {
			_playlistsChangeSignal.signal();
		}

		if (finishedWithEntity) {
			_refreshing = false;

#pragma message("TODO: don't remove songs AND playlists if saved for offline!")
	        IApp::instance()->db()->remove(entity, 
                                       ClientDb::Predicate("source",behaviour.sourceIdentifier),
                                       removeKey,
                                       _toRemove[entity]);

			if (entity == "Song") {
				Tracking::track(behaviour.name, "Number of Songs", _numberOfSongsBeforeFetch - _toRemove[entity].size());
			}
			_toRemove[entity].clear();
	    	_toRemoveStartedCounting[entity] = false;
		}
	}

    void method playbackData(const ISong &song, const function<void(const shared_ptr<IPlaybackData> &)> data) const
    {
        behaviour.playback(song.uniqueId(), data);
    }

    shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<ISong> &song) const
    {
        return shared_ptr<Gui::IPaintable>(new Gui::NamedImage(behaviour.icon));
    }

    shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<IPlaylist> &song) const
    {
        return shared_ptr<Gui::IPaintable>(new Gui::NamedImage(behaviour.icon));
    }

    vector<shared_ptr<IPlaylist>> method playlists()
    {
        auto playlistData = IApp::instance()->db()->fetchSync(
                    "Playlist", 
                    ClientDb::Predicate(ClientDb::Predicate::Operator::And,
                                    {ClientDb::Predicate("category","playlist"),
                                     ClientDb::Predicate("source", sessionIdentifier())
                                    }),
                    ClientDb::SortDescriptor({"category","name"},true),
                    0,0,false);
        return map(playlistFromData, playlistData);
    }

    void method removePlaylist(const shared_ptr<IPlaylist> &playlist)
    {
#pragma message("TODO: remove playlist")
    }

    void method search(const std::string &filter)
    {
        auto source = behaviour.sourceIdentifier;

#ifdef DEBUG
        std::cout << "search for '" << filter << "' on " << source << " just started\n";
#endif
        auto fetchSearch = behaviour.fetchSearch;
        auto self = shared_from_this();
        Io::get().dispatch([source,filter,fetchSearch,self,this]{

            assert(Io::isCurrent());
            if (filter == _currentFilter) {
                return;
            }
            _currentFilter = filter;

            auto removeOld = [source]{
                IApp::instance()->db()->remove("Song", 
                                               ClientDb::Predicate(ClientDb::Predicate::Operator::And,
                                                    {ClientDb::Predicate("source",source),
                                                     ClientDb::Predicate("playlist","temp:free")}));
            };

            if (filter.empty()) {
                removeOld();
                return;
            }

            if (fetchSearch) {
                Json::Value context;
                context["filter"] = filter;
                context["relevance"] = 0;
                fetchSearch(filter,context,[filter,removeOld,source,self,this](const Json::Value &aContext,const vector<Json::Value> &songs)->Json::Value {
                    
                    assert(Io::isCurrent());
                    if (_currentFilter != aContext["filter"].asString()) {
                        return Json::nullValue;
                    }
                    auto context = aContext;
                    auto relevance = context["relevance"].asInt();
                    
                    if (relevance == 0) {
                        removeOld();
                    }

    #ifdef DEBUG
                    std::cout << "search for '" << filter << "' on " << source << ", got " << songs.size() << " songs\n";
    #endif
                    for (auto item : songs) {
                        item["source"] = source;
                        item["playlist"] = "temp:free";
                        // there was a separate 'relevance' field in the old version, but don't remember why was it such a good idea..
                        item["position"] = relevance;
                        ++relevance;
                        IApp::instance()->db()->upsert("Song",item);
                    }

                    context["relevance"] = relevance;
                    return context;
                });
            }
        });
    }

    static void removeDynamicSongsExceptPlaylists(const vector<string> &playlists)
    {
        auto playlistPredicates = sfl::map([](const string &p){return ClientDb::Predicate("playlist",p);},playlists);
        auto predicate = ClientDb::Predicate::compositeAnd(
                            ClientDb::Predicate("dynamic", 1),
                            ClientDb::Predicate::compositeNot(ClientDb::Predicate(ClientDb::Predicate::Operator::Or,playlistPredicates)));
        IApp::instance()->db()->remove("Song", predicate);
    }

    void method clearDynamic(const string &playlistId)
    {
        // rather remove all
        removeDynamicSongsExceptPlaylists(vector<string>());
    }

    void method fetchDynamic(const string &playlistId)
    {
        if (behaviour.fetchDynamic) {
            auto source = behaviour.sourceIdentifier;
            behaviour.fetchDynamic(playlistId,[source,playlistId](const vector<Json::Value> &songs){
                auto predicate = ClientDb::Predicate(ClientDb::Predicate::Operator::And,
                                            {ClientDb::Predicate("source",source),
                                             ClientDb::Predicate("playlist",playlistId)});

                auto db = IApp::instance()->db();
                auto order = db->max("Song", predicate, "position") +1;
                for (auto song : songs) {
                    song["position"] = order;
                    song["playlist"] = playlistId;
                    song["dynamic"] = true;
                    song["source"] = source;
                    //we need to generate own entryid otherwise there will be chaos when duplicates come (more likely in instant mix case)
                    song["entryId"] = random_string(10);
                    ++order;

                    IApp::instance()->db()->upsert("Song",song);
                }

                // little cleanup:
                std::vector<string> keepPlaylists;
                keepPlaylists.push_back(playlistId);
                auto currentSong = IApp::instance()->player()->songEntryConnector().value();
                if (currentSong) {
                    keepPlaylists.push_back(currentSong.song()->stringForKey("playlist"));
                }
#ifdef DEBUG
                //std::cout << "removing radio songs except " << intercalate(std::string(","), keepPlaylists) << std::endl;
#endif
                removeDynamicSongsExceptPlaylists(keepPlaylists);
            });
        }
    }
    
    void method createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result)
    {
        #pragma message("TODO: create user playlists")
    }

}
