//
//  YouTubeSession.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/4/13.
//
//

#include <sstream>
#include <iostream>
#include "YouTubeSession.h"
#include "PlaylistCategory.h"
#include "OAuthConnection.h"
#include "YouTubeSession.h"
#include "json.h"
#include "IWebWindow.h"
#include "Async.h"
#include "MainExecutor.h"
#include "IoService.h"
#include "YouTubeService.h"
#include "SongEntry.h"
#include "sfl/Prelude.h"
#include "sfl/Vector.h"

//static const std::string kApiKey = "AIzaSyBsFTPCLyULGO6pYZrzqcBHbRTwac6GUpc";

static const std::string kClientId = "1007807607350.apps.googleusercontent.com";
static const std::string kSecret = "uRn6drhBKkEFpbm5KtxunA8N";


namespace Gear
{
#define method YouTubeSession::

    using namespace sfl;
    
    static weak_ptr<YouTubeSession> _activeSession;
    
    DbSession::Behaviour method createBehaviour(YouTubeSession *nself)
    {
        DbSession::Behaviour behaviour;
        behaviour.name = "YouTube Personal";
        behaviour.fetchLibrary = [=](const function<bool(const string &,const vector<Json::Value> &,bool)> &callback){
            auto self = nself->shared_from_this();
           
            struct PlaylistInfo
            {
                PlaylistInfo(const string &aPlaylistId, const string &aTitle, bool aRemovable) :
                    playlistId(aPlaylistId),
                    title(aTitle),
                    removable(aRemovable)
                {
                }
                
                string playlistId;
                string title;
                bool removable;
            };
            std::map<int,std::vector<PlaylistInfo>> playlistInfos;
            
            /*auto addPlaylistMetadata = [&](int categoryTag, const string &callUrl, const string &prefix, bool removable){
                
                vector<pair<string,string>> got = self->fetchPlaylistIdsAndNames(callUrl, prefix);
                transform(got.begin(), got.end(), std::back_inserter(playlistInfos[categoryTag]), [removable](const pair<string,string> &p){
                    return PlaylistInfo(p.first, p.second, removable);
                });
            };*/
            
            vector<string> userPlaylists;
            auto callAsync = self->callAsync();

            callAsync("https://www.googleapis.com/youtube/v3/channels?part=contentDetails&mine=true&maxResults=50",
                [=](const string &firstResult){
                Json::Reader reader;
                Json::Value json;
                
                bool success = reader.parse(firstResult, json);
                if (success) {
                    auto items = json.get("items", Json::arrayValue);
                    if (items.size() > 0) {
                        auto item = items[0];
                        auto related = items[0].get("contentDetails",Json::objectValue).get("relatedPlaylists",Json::objectValue);
                        self->_favoritesId = related.get("favorites","").asString();
                        auto likes = related.get("likes","").asString();
                        auto uploads = related.get("uploads","").asString();
                        auto watchHistory = related.get("watchHistory","").asString();
                        auto watchLater = related.get("watchLater","").asString();
                        
                        auto create = [&](const string &playlistId, const string &playlistName){
                            
                            auto ret = self->fetchPlaylist(playlistId, playlistName);
                            return ret;
                        };
                        
                        self->fetchPlaylistItems(self->_favoritesId,[=](vector<Json::Value> favoriteSongs){
                            self->fetchPlaylistItems(likes,[=](vector<Json::Value> likedSongs) mutable {
                                for (auto it = likedSongs.begin() ; it != likedSongs.end() ; ++it) {
                                    // by definition
                                    (*it)["rating"] = 5;
                                    (*it)["playlist"] = "all";
                                }
                                for (auto it = favoriteSongs.begin() ; it != favoriteSongs.end() ; ++it) {
                                    (*it)["playlist"] = "all";
                                }
                                callback("Song", favoriteSongs, false);
                                callback("Song", likedSongs, false);

                                self->fetchPlaylists(callback, [self,callback]{
                                    self->fetchSubscriptions(callback,[callback]{
                                        callback("Song", vector<Json::Value>(), true);
                                        callback("Playlist", vector<Json::Value>(), true);
                                    });
                                });
#pragma message("TODO: don't duplicate items that are both favorite and liked")
                            });
                        });
                        

                        #pragma message("TODO: add playlist items to favorites (and to liked?)")
                        #pragma message("TODO: watch later, positives, and uploads list, just like before")
                        // this was commented a long ago: _positives->removeSongsOtherThanLocally(likedSongs);
                        //_positives->addSongsLocally(likedSongs);
                        // this was commented a long ago:_favorites->removeSongsOtherThanLocally(favoriteSongs);
                        //_favorites->addSongsLocally(favoriteSongs);
                        
                        /*
                        auto watchLaterList = create(watchLater, "Watch Later");
                        watchLaterList->setRemovable(false);
                        auto uploadsList = create(uploads, "My Uploads");
                        uploadsList->setRemovable(false);
                        modifyCategoryByTag(kAutoPlaylistsTag, [&](Gear::PlaylistCategory &category){
                        
                            auto &autoPlaylists = category.playlists();
                            autoPlaylists.push_back(_positives);
                            autoPlaylists.push_back(watchLaterList);
                            autoPlaylists.push_back(uploadsList);
                        });*/
                        //auto positives = fetchPositive();
                        //_positives->addSongsLocally(positives);
                    }
                }
                return;
            });
        };
        behaviour.icon = "favicon-youtube";
        behaviour.sourceIdentifier = "youtube";
        behaviour.connect = [=](const string &user, const string &pass, const function<void(bool)> &callback){
            
            auto self = nself->shared_from_this();
            callback(self->_connection->connectSync());
        };
        behaviour.presence = []{
            Json::Value ret;
            ret["all"] = "My Library";
            ret["playlist"] = "Playlists";
            ret["subscription"] = "Subscriptions";
            return std::move(ret);
        };
        behaviour.playback = YouTubeSessionBase::playbackData;
        behaviour.fetchSearch = [](const string &filter,const Json::Value &context,const DbSession::Behaviour::FetchSearchCallback &callback){
            // nada
        };
        behaviour.removePlaylist = [nself](const string &playlistId, const std::map<string,string> &traits){
            nself->removePlaylist(playlistId);
        };

        return behaviour;
    }

    static void gotDataForPlaylist(const DbSession::Behaviour::FetchCallback &callback, const string &category, const string &playlistId, const string &playlistName, const vector<Json::Value> &aSongs)
    {
        Json::Value list;
        list["category"] = category;
        list["name"] = playlistName;
        list["playlistId"] = playlistId; 
        callback("Playlist", sfl::Vector::singleton(list), false);
        auto songs = aSongs;
        int position = 1;
        for (auto &song : songs) {
            song["playlist"] = playlistId;
            song["position"] = position;
            ++position;
        }
        callback("Song", songs, false);
    }

    void method fetchPlaylists(const DbSession::Behaviour::FetchCallback &callback, const function<void()> &finished)
    {
        #pragma message("TODO: don't forget to set position for playlists")

        // user's own playlists
        auto self = shared_from_this();
        fetchPlaylistIdsAndNames("https://www.googleapis.com/youtube/v3/playlists?part=id,snippet&mine=true&maxResults=50","",[self,this,callback,finished](const vector<pair<string,string>> &gotPlaylists){
            
            auto gotDataPlaylist = [callback](const string &playlistId, const string &playlistName, const vector<Json::Value> &aSongs){
                return gotDataForPlaylist(callback, "playlist", playlistId, playlistName, aSongs);
            };

            fetchPlaylistsForCategory("playlist",filter([](const std::pair<string,string> &p){return p.second != "Favorites";},gotPlaylists),
                           gotDataPlaylist, finished); 
        });
    }

    void method fetchSubscriptions(const DbSession::Behaviour::FetchCallback &callback, const function<void()> &finished)
    {
        auto self = shared_from_this();
        callAsync("https://www.googleapis.com/youtube/v3/subscriptions?part=snippet&mine=true&maxResults=50",
                  "GET",
                  "",
                  std::map<string,string>(),
                  [finished,callback,self](const string &result,int statusCode){

            Json::Reader reader;
            Json::Value json;
            bool success = reader.parse(result, json);
            if (!success) {
                finished();
            }

            using std::pair;
            using std::make_pair;
            
            // note: we can optimize here to only fetch results which are not known from an older time, if we are low on quota
            auto items = json.get("items", Json::arrayValue);
            vector<pair<string,string>> channelIds;
            for (auto it = items.begin() ; it != items.end() ; ++it) {
                auto &item = *it;
                
                auto snippet = item.get("snippet", Json::objectValue);
                auto title = snippet.get("title","").asString();
                auto channelId = snippet.get("resourceId", Json::objectValue).get("channelId", "").asString();
                if (!channelId.empty()) {
                    channelIds.push_back(make_pair(channelId, title));
                }
            }

            auto gotDataSubscriptions = [callback](const string &playlistId, const string &playlistName, const vector<Json::Value> &aSongs){
                return gotDataForPlaylist(callback, "subscription", playlistId, playlistName, aSongs);
            };
            self->fetchSubscriptions(channelIds, gotDataSubscriptions, finished);
        });
    }   

    void method fetchSubscriptions(const vector<pair<string,string>> &channelIdsAndTitles, const function<void(const string &, const string &,const vector<Json::Value> &)> &gotData, const function<void()> &finished)
    {
        if (channelIdsAndTitles.empty()) {
            finished();
            return;
        }

        auto it = channelIdsAndTitles.begin();
        const string &channelId = it->first;
        const string &channelTitle = it->second;
        auto self = shared_from_this();

        fetchPlaylistIdsAndNames("https://www.googleapis.com/youtube/v3/playlists?part=id,snippet&maxResults=50&channelId=" + channelId, 
                                 channelTitle + " - ",
                                 [channelIdsAndTitles,this,self,gotData,finished](const std::vector<pair<string,string>> &idsAndNames){

            fetchPlaylistsForCategory("subscription", idsAndNames, gotData, [channelIdsAndTitles,gotData,this,self,finished]{
                fetchSubscriptions(tail(channelIdsAndTitles), gotData, finished);
            });
        });   
    }

    void method fetchPlaylistsForCategory(const string &category, const vector<pair<string,string>> &idsAndNames, const function<void(const string &, const string &,const vector<Json::Value> &)> &gotData, const function<void()> &finished)
    {
        if (idsAndNames.empty()) {
            finished();
            return;
        }

        auto it = idsAndNames.begin();
        const string &playlistId = it->first;
        const string &playlistName = it->second;
        auto self = shared_from_this();
        fetchPlaylistItems(playlistId, [playlistId,playlistName,gotData,self,this,category,idsAndNames,finished](const vector<Json::Value> &songs){
            gotData(playlistId, playlistName, songs);
            fetchPlaylistsForCategory(category, tail(idsAndNames), gotData, finished);
        });
    }

    shared_ptr<YouTubeSession> method create(const shared_ptr<IApp> &app)
    {
        auto ret = shared_ptr<YouTubeSession>(new YouTubeSession(app));
        
        ret->resetConnection();
        
        _activeSession = ret;
        
        return ret;
    }

    static string identifier()
    {
        return YouTubeService::staticIdentifier();
    }

    void method resetConnection()
    {
//#ifndef TIZEN
//      string callback = "http://localhost";
        string callback = "urn:ietf:wg:oauth:2.0:oob";
//#else
//      string callback = "http://www.local.com/oauth2callback";
//#endif
        _connection = OAuthConnection::create("https://accounts.google.com/o/oauth2", kClientId, kSecret, "https://www.googleapis.com/auth/youtube", callback, identifier());
    }
    
    shared_ptr<YouTubeSession> method loggedInSession()
    {
        return shared_from_this();
    }
    
    shared_ptr<YouTubeSession> method activeSession()
    {
        auto s = _activeSession.lock();
        return s;
    }
    
   
    /*
    void method createCategories()
    {
        vector<PlaylistCategory> categories;
        
        PlaylistCategory all = PlaylistCategory(kAllCategory, kAllTag, true);
        
        all.playlists().push_back(_favorites);
        categories.push_back(all);
        
        PlaylistCategory playlists = PlaylistCategory(kPlaylistsCategory, kPlaylistsTag);
        categories.push_back(playlists);
        
        PlaylistCategory subscriptions = PlaylistCategory(kSubscriptionsCategory, kSubscriptionsTag);
        categories.push_back(subscriptions);
        
        PlaylistCategory autoPlaylists = PlaylistCategory(kAutoPlaylistsCategory, kAutoPlaylistsTag);
        // this is not necessary, as sessionmanager manages this for us
        //autoPlaylists.playlists().push_back(PredicatePlaylist::thumbsDownPlaylist(_favorites->storedSongArray()));
        categories.push_back(autoPlaylists);
        
        _categories = categories;
    }*/

    method YouTubeSession(const shared_ptr<IApp> &app) :
        DbSession(app,createBehaviour(this))
    {
    }
    
    /*
    string method callSync(const string &str)
    {
        return _connection->callSync(str);
    }
    
    string method callSync(const string &str, const string &m, const string &body, int *resultCode)
    {
        map<string, string> headers;
        headers.insert(std::make_pair("Content-Type","application/json"));
        return _connection->callSync(str, m, body, headers, resultCode);
    }*/

    YouTubeSessionBase::CallAsync method callAsync()
    {
        auto self = shared_from_this();
        return [self,this](const string &request, const function<void(const string &)> &result){
            assert(Io::isCurrent());

            _connection->callAsync(request, "GET", "", std::map<string,string>(), [result](const string &str,int){
                result(str);
            });
        };
    }

    void method callAsync(const std::string &request, const std::string &m, const std::string &body, const std::map<string,string> &headers, const function<void(const string &, int)> &result)
    {
        _connection->callAsync(request, "GET", "", std::map<string,string>(), [result](const string &str,int code){
            result(str, code);
        });
    }

    void method fetchPositive(const function<void(const vector<Json::Value> &)> &result)
    {
        // note that we could optimize here by fetching snippet and contentDetails
        
        std::stringstream ss;
        ss << "https://www.googleapis.com/youtube/v3/videos?part=id&maxResults=50&myRating=like";
        
        YouTubeSessionBase::callForSongs(callAsync(), ss.str(), result);
    }
    
    /*
    shared_ptr<IPlaylist> method libraryPlaylist()
    {
    	// why would this be true?
    	// return _positives;
        return _favorites;
    }*/
    
    shared_ptr<YouTubeSession> method shared_from_this()
    {
        return MEMORY_NS::static_pointer_cast<YouTubeSession>(DbSession::shared_from_this());
    }
    
    void method fetchPlaylistItems(const string &playlistId, const function<void(const vector<Json::Value> &)> &result)
    {
        YouTubeSessionBase::callForSongsCommon(callAsync(), playlistId, "https://www.googleapis.com/youtube/v3/playlistItems?part=snippet,contentDetails,status&maxResults=50&playlistId=" + playlistId, result);
    }
    
    void method fetchPlaylistIdsAndNames(const string &callUrl, const string &titlePrefix, const function<void(const vector<pair<string,string>> &)> &callback)
    {
        callAsync()(callUrl, [titlePrefix,callback](const string &result){

            Json::Reader reader;
            Json::Value json;
            
            vector<pair<string,string>> ret;
            
            bool success = reader.parse(result, json);
            if (!success) {
                callback(ret);
                return;
            }
            
            auto items = json.get("items", Json::arrayValue);
            for (auto playlistIt = items.begin() ; playlistIt != items.end() ; ++playlistIt) {
                auto &playlist = *playlistIt;
                string playlistId = playlist.get("id","").asString();
                string title = playlist.get("snippet", Json::objectValue).get("title","").asString();
                if (!playlistId.empty()) {
                    std::stringstream fullTitle;
                    fullTitle << titlePrefix << title;
                    ret.push_back(std::make_pair(playlistId,fullTitle.str()));
                }
            }
        
            callback(ret);
        });
    }
    
    void method addUserPlaylistLocally(const shared_ptr<ModifiablePlaylist> playlist)
    {
        #pragma message("TODO: add user playlist")
        /*
        modifyCategoryByTag(kPlaylistsTag, [&](PlaylistCategory &category){
            auto &playlists = category.playlists();
            playlists.push_back(playlist);
        });
        _userPlaylists.set([&](vector<shared_ptr<ModifiablePlaylist>> &userPlaylists){
            
            userPlaylists.push_back(playlist);
        });
        _playlistsChangeSignal.signal();*/
    }
    
    ValidPtr<const vector<PlaylistCategory>> method categories() const
    {
        return _categories;
    }
    
    /*
    void method fetchRatings(const vector<SongEntry> &entries)
    {
        std::stringstream ss;
        ss << "https://www.googleapis.com/youtube/v3/videos/getRating?id=";
        bool first = true;
        std::map<string,shared_ptr<ISong>> entriesMap;
        for (auto it = entries.begin() ; it != entries.end() ; ++it) {
            auto &entry = *it;
            string entryId = entry.song()->uniqueId();
            entriesMap.insert(std::make_pair(entryId, entry.song()));
            if (first) {
                first = false;
            } else {
                ss << ",";
            }
            ss << entryId;
        }
        auto str = ss.str();
        auto result = callSync(str);
        Json::Value json;
        Json::Reader reader;
        if (reader.parse(result, json)) {
            auto items = json.get("items", Json::arrayValue);
            for (auto it = items.begin() ; it != items.end() ; ++it) {
                auto &item = *it;
                auto videoId = item.get("videoId","").asString();
                auto rating = item.get("rating","").asString();
                int ratingNum = 0;
                if (rating == "like") {
                    ratingNum = 5;
                } else if (rating == "dislike") {
                    ratingNum = 1;
                }
                auto itSong = entriesMap.find(videoId);
                if (itSong != entriesMap.end()) {
                    itSong->second->setUintForKey("rating", ratingNum);
                }
            }
        }
    }*/
    
#pragma message("TODO: add song to youtube playlist")
#if 0
    void method doAddToPlaylist(const shared_ptr<ModifiablePlaylist> &playlist, const shared_ptr<ISong> &song, const function<void(const vector<SongEntry> &)> &result)
    {
        doAddToPlaylist(playlist, song, playlist->playlistId(), result);
    }
    
    void method doAddToPlaylist(const shared_ptr<ModifiablePlaylist> &playlist, const shared_ptr<ISong> &song, const string &playlistId, const function<void(const vector<SongEntry> &)> &result)
    {
        auto self = shared_from_this();
        tryWithUserChannel([self,this,playlistId,song,playlist,result](const function<void()> &ifNeedsChannel){
            Json::Value value;
            value["snippet"]["playlistId"] = playlistId;
            value["snippet"]["resourceId"]["videoId"] = song->uniqueId();
            value["snippet"]["resourceId"]["kind"] = "youtube#video";
            
            Json::FastWriter writer;
            auto body = writer.write(value);
            int resultCode = 0;
            callAsync("https://www.googleapis.com/youtube/v3/playlistItems?part=snippet", "POST", body, std::map<string,string>(), [ifNeedsChannel,result,song](const string &resultStr, int resultCode){

                if (resultCode == 401) {
                    if (ifNeedsChannel) {
                        ifNeedsChannel();
                    } else {
                        result(vector<SongEntry>());
                    }
                    return;
                }

                vector<SongEntry> entries;
                entries.push_back(SongEntry(song,song->uniqueId()));
    #pragma message("TODO: add to local playlist when adding remotely")
                //playlist->addSongsLocally(entries);
                result(entries);
            });
        }, [result]{
            result(vector<SongEntry>());
        });
    }
#endif

#pragma message("TODO: create user playlist on youtube")
#if 0 
    void method createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &resultCallback)
    {
        auto self = shared_from_this();

        auto doCreate = [songs, name, self, resultCallback](const function<void()> &ifNeedsChannel){
            Json::Value value;
            value["snippet"]["title"] = name;
            Json::FastWriter writer;
            
            auto body = writer.write(value);

            self->callAsync("https://www.googleapis.com/youtube/v3/playlists?part=snippet", "POST", body, std::map<string,string>(), [resultCallback,ifNeedsChannel,name,self,songs](const string &result, int resultCode){

#ifdef DEBUG
    //            std::cout << "create playlist body: " << body << std::endl;
    //            std::cout << "create playlist result (" << resultCode << "): " << result << std::endl;
#endif

                if (resultCode == 401) {
                    // if there are no channel present yet, the user gets an error of "reason": "youtubeSignupRequired”
                    // we must then present a dialog about this...
                    if (ifNeedsChannel) {
                        ifNeedsChannel();
                    } else {
                        resultCallback(shared_ptr<IPlaylist>());
                    }
                    return;
                }

                Json::Reader reader;
                Json::Value value;
                if (reader.parse(result,value)) {
                    string playlistId = value.get("id","").asString();
                    if (!playlistId.empty()) {
                
#pragma message("TODO: we don't do this anymore")
                        shared_ptr<ModifiablePlaylist> playlist = ModifiablePlaylist::create(playlistId, name, self);
                        //playlist->setOrdered(true);
                        
                        playlist->setName(name);
                        
                        self->addUserPlaylistLocally(playlist);
                        
                        Async::forEach<shared_ptr<ISong>>(songs, [self,playlist](const shared_ptr<ISong> &song, const function<void()> &cont){
                            self->doAddToPlaylist(playlist, song, [cont](const vector<SongEntry> &added){
                                cont();
                            });
                        }, [playlist,resultCallback](){
                            resultCallback(playlist);
                        });
                        return;
                        //pThis->app()->editPlaylistName(playlist);
                
                    }
                }
                resultCallback(shared_ptr<IPlaylist>());
            });
            
        };

        tryWithUserChannel(doCreate, [resultCallback]{
            resultCallback(shared_ptr<IPlaylist>());
        });
    }
#endif

    void method tryWithUserChannel(const function<void(const function<void()> &)> &doCreate, const function<void()> &aFail)
    {
        // handle the case when there is no channel associated with the youtube user:
        auto self = shared_from_this();
        auto success = [self,this,doCreate](const function<void()> &dispose){
            dispose();
            _executor.addTask([doCreate]{
                doCreate(function<void()>());
            });
        };
        auto fail = [self,this,aFail](const function<void()> &dispose){
            dispose();
            _executor.addTask(aFail);
        };

        auto proceed = [success,fail](const shared_ptr<IWebWindow> &window, const function<void()> &dispose){
#ifdef DEBUG
//            std::cout << "youtube channel link proceed '" << window->url() << "'" << std::endl;
#endif
            if (!window->visible()) {
#ifdef DEBUG
//                std::cout << "youtube channel link fail";
#endif
                fail(dispose);
                return;
            } 

            // it's https://www.youtube.com/channel_creation_done?reload, not https://m.youtube.com/channel_creation_done
            if (window->url().find("youtube.com/channel_creation_done") != std::string::npos) {
#ifdef DEBUG
//                std::cout << "youtube channel link success";
#endif
                success(dispose);
                return;
            } 
        };

        _executor.addTask([doCreate,proceed,fail]{
            doCreate([doCreate,proceed,fail]{

                MainExecutor::instance().addTask([doCreate,proceed,fail]{
                    auto window = IWebWindowDelegate::create([proceed](const shared_ptr<IWebWindow> &window, const function<void()> &dispose){
                        // finishload
                        proceed(window, dispose);

                    }, [fail](const shared_ptr<IWebWindow> &window, const function<void()> &dispose){
                        // fail

                        fail(dispose);   

                    }, [proceed](const shared_ptr<IWebWindow> &window, const function<void()> &dispose){
                        // navigate

                        proceed(window, dispose);
                    }); 
                    window->show("Create YouTube Channel");
                    window->loadUrl("https://m.youtube.com/create_channel?chromeless=1&next=/channel_creation_done");
                    IWebWindow::checkPeriodically(window);
                });
            });
        });
    }
    
    void method modifySongs(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts)
    {
        auto pThis = shared_from_this();
        _executor.addTask([pThis, changedInts, ids]{
            auto ratingPresent = changedInts.find("rating");
            if (ratingPresent != changedInts.end()) {
                long long rating = ratingPresent->second;
                
                vector<SongEntry> entries;

                #pragma message("TODO: this whole thing was about inconsistency of positives, etc...")
                /*
				auto &libArray = pThis->_favorites->songArray();
				auto raw = libArray->rawSongs();
				auto &posLibArray = pThis->_positives->songArray();
				auto posRaw = posLibArray->rawSongs();
                for (auto it = ids.begin() ; it != ids.end() ; ++it) {
                    
                    std::stringstream ss;
                    ss << "https://www.googleapis.com/youtube/v3/videos/rate?id=";
                    ss << *it << "&rating=";
                    string ratingString = "none";
                    if (rating == 1 || rating == 2) {
                        ratingString = "dislike";
                    } else if (rating == 4 || rating == 5) {
                        ratingString = "like";
                    }
                    ss << ratingString;
                    string result = pThis->callSync(ss.str(), "POST", "");
                    
                    SongEntry entry(shared_ptr<ISong>(), *it);

                    if (rating != 5) {
						for (auto rawIt = posRaw->begin() ; rawIt != posRaw->end() ; ++rawIt) {
							if ((*rawIt).song()->uniqueId() == *it) {
								entries.push_back(*rawIt);
								break;
							}
						}
                    } else {
                    //if ((foundInPos && rating != 5) || (!foundInPos && rating == 5)) {
                        auto rawIt = posRaw->begin();
                        for (; rawIt != posRaw->end() ; ++rawIt) {
							if ((*rawIt).song()->uniqueId() == *it) {
								break;
							}
						}
                        if (rawIt == posRaw->end()) {
                            // don't add to positives if it's already in there
                            for (auto rawIt = raw->begin() ; rawIt != raw->end() ; ++rawIt) {
                                if ((*rawIt).song()->uniqueId() == *it) {
                                    entries.push_back(*rawIt);
                                    break;
                                }
                            }
                        }
                    }
                    //}
                }
                if (rating != 5) {
                	pThis->_positives->removeSongs(entries);
                } else {
                	pThis->_positives->addSongsLocally(entries);
                }
                */
            }
        });
    }
    
    void method addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result)
    {
#pragma message("TODO: add songs to playlist")
        /*
        auto pThis = shared_from_this();
        vector<shared_ptr<ModifiablePlaylist>> userPlaylists = _userPlaylists;
        auto it = find_if(userPlaylists.begin(), userPlaylists.end(), [&](const shared_ptr<ModifiablePlaylist> &rhs){
            return rhs->playlistId() == playlistId;
        });
        if (it != userPlaylists.end()) {
            auto &playlist = *it;
            
            addSongsToPlaylistAsync(playlist, songs, result);
            return;
        }

        vector<SongEntry> ret;
        result(ret);*/
    }

#if 0
    void method addSongsToPlaylistAsync(const shared_ptr<ModifiablePlaylist> &playlist, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result)
    {
        addSongsToPlaylistAsync(playlist, songs, playlist->playlistId(), result);
    }
    
    void method addSongsToPlaylistAsync(const shared_ptr<ModifiablePlaylist> &playlist, const vector<shared_ptr<ISong>> &songs, const string &playlistId, const function<void(const vector<SongEntry> &)> &result)
    {
        auto self = shared_from_this();
        Async::forEach<shared_ptr<ISong>,vector<SongEntry>>(songs, [self,playlist,playlistId](const shared_ptr<ISong> &song, vector<SongEntry> &ret, const function<void(const vector<SongEntry> &r)> &cont){
            self->doAddToPlaylist(playlist, song, playlistId, [ret,cont](const vector<SongEntry> &added) mutable {
                copy(added.begin(),added.end(),back_inserter(ret));
                cont(ret);
            });
        }, [result](const vector<SongEntry> &ret){
            result(ret);
        });
    }
#endif
    
    void method addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
    {
    	auto self = shared_from_this();
#pragma message("TODO: add youtube search to library")
        /*
    	_outOfOrder.addTask([self,songs] () {
            

            auto favorites = self->_favorites;
            vector<shared_ptr<ISong>> songsFiltered;
            remove_copy_if(songs.begin(), songs.end(), back_inserter(songsFiltered), [&](const shared_ptr<ISong> &song){
                bool contains = favorites->songArray()->contains(SongEntry(song, song->uniqueId()));
                return contains;
            });
            if (songsFiltered.empty()) {
                return;
            }
    		self->addSongsToPlaylistAsync(favorites, songs, self->_favoritesId,[self,songs](const vector<SongEntry> &result){
                // add to positives if necessary (ignore the result)
                vector<SongEntry> entries;
                for (auto it = songs.begin() ; it != songs.end() ; ++it) {
                    auto &song = *it;
                    if (song->rating() == 5) {
                        entries.push_back(SongEntry(song, song->uniqueId()));
                    }
                }
                if (entries.size() > 0) {
                    self->_positives->addSongsLocally(entries);
                }
            });
    	});
        */
    }
    
    void method removePlaylist(const string &playlist)
    {
        callAsync("https://www.googleapis.com/youtube/v3/playlists?id=" + playlist, "DELETE", "", std::map<string,string>(), [](const string &result,int){
        });
    }
    
    void method deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId)
    {
        #pragma message("TODO: remove from positives and favorites as well! different entry ids, so we must search back...")
        // remove from positives and favorites as well! different entry ids, so we must search back...
        /*auto removeSongIdFromList = [](const string &songId, IPlaylist &playlist){
        	auto rawPtr = playlist.songArray()->rawSongs();
            auto &raw = *rawPtr;
			vector<SongEntry> toRemove;
        	for (auto it = raw.begin() ; it != raw.end() ; ++it) {
        		auto &entry = *it;
        		if (entry.song()->uniqueId() == songId) {
        			auto entryId = entry.entryId();
        			toRemove.push_back(SongEntry(shared_ptr<ISong>(), entryId));
        			break;
        		}
        	}
			if (toRemove.size() > 0) {
				playlist.removeSongs(toRemove);
			}
        };

        if (playlistId == "all") {
        	for (auto it = songIds.begin() ; it != songIds.end() ; ++it) {
        		auto &songId = *it;
        		removeSongIdFromList(songId, *_favorites);
        		removeSongIdFromList(songId, *_positives);
        	}
        }*/

        struct Inner
        {
            static void doDelete(const shared_ptr<YouTubeSession> &self, const vector<std::string> &entryIds)
            {
                if (!entryIds.empty()) {
                    self->callAsync("https://www.googleapis.com/youtube/v3/playlistItems?id=" + head(entryIds), "DELETE", "", std::map<string,string>(), [self,entryIds](const string &,int){
                        doDelete(self,tail(entryIds));
                    });
                }
            }
        };

        auto self = shared_from_this();
        Inner::doDelete(self,entryIds);
    }
    
    void method changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before)
    {
        // TODO
    }
    
    void method changePlaylistNameSync(const string &newName, const string &playlistId)
    {
        auto self = shared_from_this();
        callAsync("https://www.googleapis.com/youtube/v3/playlists?part=snippet&id=" + playlistId, "GET", "", std::map<string,string>(), [newName,self](const string &result,int){
            
            Json::Reader reader;
            Json::Value value;
            reader.parse(result, value);
            const auto items = value.get("items",Json::arrayValue);
            if (items.size() < 1) {
                return;
            }
            auto playlist = items[0];
            if (playlist.isNull()) {
                return;
            }
            auto &snippet = playlist["snippet"];
            if (snippet.isNull()) {
                return;
            }
            snippet["title"] = newName;

            Json::FastWriter writer;
            self->callAsync("https://www.googleapis.com/youtube/v3/playlists?part=snippet", "PUT", writer.write(playlist), std::map<string,string>(),[playlist](const string &,int){
#ifdef DEBUG
                Json::FastWriter writer;
                std::cout << "modifycall: " << writer.write(playlist) << std::endl;
#endif
            });
        });
    }
    
    vector<int> method possibleRatings() const
    {
        std::vector<int> ret;
        ret.push_back(0);
        ret.push_back(1);
        ret.push_back(5);
        return ret;
    }
};
