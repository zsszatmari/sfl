//
//  YouTubeSearchSession.cpp
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/8/13.
//
//

#include <sstream>
#include "YouTubeSearchSession.h"
#include "PlaylistCategory.h"
#include "BasicPlaylist.h"
#include "UrlEncode.h"
#include "HttpDownloader.h"
#include "YouTubeSession.h"
#include "YouTubeSessionBase.h"
#include "IoService.h"
#include "AsyncHttp.h"

namespace Gear
{
#define method YouTubeSearchSession::
    
    // when accessing resources without the need of authentication:
    static const std::string kApiKey = "AIzaSyBsFTPCLyULGO6pYZrzqcBHbRTwac6GUpc";
    
    shared_ptr<YouTubeSearchSession> method create(const shared_ptr<IApp> &app)
    {
        auto ret = shared_ptr<YouTubeSearchSession>(new YouTubeSearchSession(app));
        return ret;
    }

    static YouTubeSessionBase::CallAsync callAsync()
    {
        return [](const string &request, const function<void(const string &)> &result){
            assert(Io::isCurrent());

            string url = request + "&key=" + kApiKey;

            AsyncHttp::Request req(url);
            AsyncHttp::download(Io::get(), req, [result](const AsyncHttp::Response &response){
                if (response.ok()) {
                    result(response.bodyAsString());
                } else {
                    result("");
                }
            });
        };
    }
    
    static void searchAsync(const string &filter, const Json::Value &context, const DbSession::Behaviour::FetchSearchCallback &callback)
    {
        auto page = context["user"]["page"].asInt();
        if (page >= 5) {
            return;
        }

        auto token = context["user"]["token"].asString();
        
        std::stringstream ss;
        ss << "https://www.googleapis.com/youtube/v3/search?part=id&maxResults=50&type=video&q=" << UrlEncode::encode(filter);
        
        YouTubeSessionBase::callForSongs(callAsync(), ss.str(), token, [filter,context,page,callback](const vector<Json::Value> &ret, const string &token) mutable {
            auto newContext = context;
            newContext["user"]["token"] = token;
            ++page;
            newContext["user"]["page"] = page;
            searchAsync(filter,callback(newContext,ret),callback);
        });
    }
    
    DbSession::Behaviour method createBehaviour(YouTubeSearchSession *nself)
    {
        DbSession::Behaviour behaviour;
        behaviour.icon = "favicon-youtube";
        behaviour.name = "YouTube Search";
        behaviour.sourceIdentifier = "youtube-search";
        behaviour.fetchLibrary = [](const function<bool(const string &,const vector<Json::Value> &,bool)> &callback){
        };
        behaviour.connect = [=](const string &user, const string &pass, const function<void(bool)> &callback){
            callback(true);
        };
        behaviour.presence = []{
            Json::Value ret;
            ret["free"] = "Search";
            return std::move(ret);
        };
        behaviour.playback = YouTubeSessionBase::playbackData;
        behaviour.fetchSearch = [nself](const string &filter, const Json::Value &context, const DbSession::Behaviour::FetchSearchCallback &callback){
            searchAsync(filter, context, callback);
        };
     
        return behaviour;
    }

    method YouTubeSearchSession(const shared_ptr<IApp> &app) :
        DbSession(app,createBehaviour(this)),
        lastSearchRepeat(0)
    {
    }
    
    static shared_ptr<YouTubeSession> loggedInSession()
    {
        return YouTubeSession::activeSession();
    }

    void method addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
    {
        auto active = loggedInSession();
        if (active) {
            active->addAllAccessToLibrary(songs);
        }
    }

    ValidPtr<const vector<PlaylistCategory>> method categories() const
    {
        return _categories;
    }
                                                    
    shared_ptr<IPlaylist> method libraryPlaylist()
    {
        return shared_ptr<IPlaylist>();
    }
    
    vector<shared_ptr<IPlaylist>> method playlists()
    {
        auto active = loggedInSession();
        if (active) {
            return active->playlists();
        }
        return vector<shared_ptr<IPlaylist>>();
    }
    
    void method createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result)
    {
#pragma message("TODO: create user playlist from youtube search result")
        /*
        auto active = loggedInSession();
        if (active) {
            active->createUserPlaylist(songs, name, result);
        } else {
            result(shared_ptr<IPlaylist>());
        }*/
    }
    
    void method modifySongs(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts)
    {
        auto active = loggedInSession();
        if (active) {
            active->modifySongs(ids, changedStrings, changedInts);
        }
    }
    
    void method addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result)
    {
        auto active = loggedInSession();
        if (active) {
            active->addSongsToPlaylistAsync(playlistId, songs, result);
            return;
        }
        result(vector<SongEntry>());
    }
    
    void method deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId)
    {
        auto active = loggedInSession();
        if (active) {
            active->deletePlaylistEntriesSync(entryIds, songIds, playlistId);
        }
    }
    
    bool method manipulationEnabled() const
    {
    	auto active = loggedInSession();
    	if (active) {
    		return true;
    	}
    	return false;
    }

    void method changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before)
    {
        auto active = loggedInSession();
        if (active) {
            active->changePlaylistOrderSync(playlistId, songs, afterId, before);
        }
    }
    
    void method changePlaylistNameSync(const string &newName, const string &playlistId)
    {
        auto active = loggedInSession();
        if (active) {
            active->changePlaylistNameSync(newName, playlistId);
        }
    }
    
    vector<int> method possibleRatings() const
    {
        auto active = loggedInSession();
        if (active) {
            return active->possibleRatings();
        } else {
            vector<int> ret;
            ret.push_back(0);
            return ret;
        }
    }
}
