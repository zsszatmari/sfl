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
#include "FreeSearchArray.h"
#include "BasicPlaylist.h"
#include "UrlEncode.h"
#include "HttpDownloader.h"
#include "YouTubeSession.h"

namespace Gear
{
#define method YouTubeSearchSession::
    
    // when accessing resources without the need of authentication:
    static const std::string kApiKey = "AIzaSyBsFTPCLyULGO6pYZrzqcBHbRTwac6GUpc";
    
    shared_ptr<YouTubeSearchSession> method create(const shared_ptr<IApp> &app)
    {
        auto ret = shared_ptr<YouTubeSearchSession>(new YouTubeSearchSession(app));
        
        vector<PlaylistCategory> categories;
        PlaylistCategory all(kAllCategory, kAllTag, true);
        all.setSingularPlaylist(false);
        categories.push_back(all);
        
        auto &allCategoryLists = categories.at(0).playlists();
        auto freeSearch = FreeSearchArray::create(ret);
        allCategoryLists.push_back(shared_ptr<IPlaylist>(new BasicPlaylist(ret, "Search", "free", shared_ptr<ISongArray>(freeSearch))));
        
        ret->_categories = categories;
        
        return ret;
    }
    
    method YouTubeSearchSession(const shared_ptr<IApp> &app) :
        YouTubeSessionBase(app),
        lastSearchRepeat(0)
    {
    }
                                                    
    string method callSync(const string &str)
    {
        string url = str + "&key=" + kApiKey;
        std::map<string, string> headers;
        
        string m = "GET";
        string body;
        
        auto downloader = HttpDownloader::create(url, headers, m, body);
        downloader->waitUntilFinished();
        //auto fail = downloader->failed();
        
        string ret = *downloader;
        
        return ret;
    }
    
    const vector<SongEntry> method searchSync(const string &filter, string &token)
    {
        if (lastSearchFilter == filter) {
            if (lastSearchRepeat >= 5) {
                return vector<SongEntry>();
            } else {
                ++lastSearchRepeat;
            }
        } else {
            lastSearchFilter = filter;
            lastSearchRepeat = 1;
            searchRelevance = 0;
        }
        
        std::stringstream ss;
        ss << "https://www.googleapis.com/youtube/v3/search?part=id&maxResults=50&type=video&q=" << UrlEncode::encode(filter);
        
        auto ret = callForSongs(ss.str(), token);
        for (auto it = ret.begin() ; it != ret.end() ; ++it) {
            it->song()->setUintForKey("relevance", searchRelevance);
            ++searchRelevance;
        }
        
        return ret;
    }
    
    void method refresh()
    {
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
        auto active = loggedInSession();
        if (active) {
            active->createUserPlaylist(songs, name, result);
        } else {
            result(shared_ptr<IPlaylist>());
        }
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
    
    void method removePlaylist(const shared_ptr<IPlaylist> &playlist)
    {
        auto active = loggedInSession();
        if (active) {
            active->removePlaylist(playlist);
        }
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
