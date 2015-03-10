//
//  GoogleMusicProtocol.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/9/13.
//
//

#ifndef __G_Ear_Player__GoogleMusicProtocol__
#define __G_Ear_Player__GoogleMusicProtocol__

#include <vector>
#include <functional>
#include "stdplus.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Json
{
    class Value;
}

namespace Gear
{
    using std::string;
    using std::vector;
    using std::function;
    
    class GooglePlaySessionImpl;
    class PlaybackData;
    class IWebWindow;
    class GoogleMusicConnection;
    class WebCookie;
    
    class GoogleMusicProtocol final
    {
    public:
        GoogleMusicProtocol(const shared_ptr<GoogleMusicConnection> &session);
        
        void setUserName(const string &name);
        const string &userName() const;
        bool loginWithCookies(const Json::Value &cookies);
        const std::vector<WebCookie> &cookiesToSave() const;
        void searchSongs(const string &filter, const function<void(const shared_ptr<Json::Value> &)> &f);
        void loadExplore(const function<void(const shared_ptr<Json::Value> &)> &f);
        shared_ptr<Gear::PlaybackData> playDirect(const string &directId);
        shared_ptr<PlaybackData> play(const string &songId);
        void fetchArtist(const string &artistId, const function<void(const shared_ptr<Json::Value> &)> &f);
        void fetchAlbum(const string &albumId, const function<void(const shared_ptr<Json::Value> &)> &f);
        void fetchRadioFeed(const string &radioId, const function<void(const shared_ptr<Json::Value> &)> &f);
        void fetchSharedSongs(const string &token, const function<void(const shared_ptr<Json::Value> &)> &f);
        void fetchRatedSongs(const function<void(const shared_ptr<Json::Value> &)> &f);
        void addSongsToLocker(const vector<string> &songIds, const function<void(const shared_ptr<Json::Value> &)> &f);
        void loadRadio(const function<void(const shared_ptr<Json::Value> &)> &f);
        void createStation(const string &songId, const string &name, int type, const function<void(const shared_ptr<Json::Value> &)> &f);
        void deleteStation(const string &radioId);
        void deletePlaylist(const string &playlistId);
        void getAllSongs(const function<bool(const Json::Value &)> &songs);
        void getListOfPlaylists(const function<void(const shared_ptr<Json::Value> &)> &callback);

        void getPlaylist(const string &playlistId, const function<void(const shared_ptr<Json::Value> &)> &f);
        //shared_ptr<Json::Value> getAllPlaylists();
        void addPlaylistEntries(const vector<string> &songs, const string &playlist, const function<void(const shared_ptr<Json::Value> &)> &f);
        void createPlaylist(const vector<string> &songIds, const string &title, const function<void(const shared_ptr<Json::Value> &)> &f);
        void deletePlaylistEntries(const vector<string> &entries, const vector<string> &songs, const string &playlist);
        void changePlaylistName(const string &name, const string &playlist);
        void changePlaylistOrder(const string &playlist, const vector<string> &movedSongs, const vector<string> &movedEntries, const string &beforeEntry, const string &afterEntry);
        void changeSongs(const Json::Value &changeDictionaries);
        
    private:
        shared_ptr<GoogleMusicConnection> _connection;
        
        bool _allAccess;
        bool _cancelling;
        
        void call(const string &name, const Json::Value &param, const function<void(const shared_ptr<Json::Value> &)> &callback);
        shared_ptr<PlaybackData> play(const string &aSongId, const string &paramName);
        static shared_ptr<Json::Value> doGetListOfPlaylists(const std::string &str);
        static shared_ptr<Json::Value> doGetListOfPlaylistsOld(const std::string &str);
    };
}

#endif /* defined(__G_Ear_Player__GoogleMusicProtocol__) */
