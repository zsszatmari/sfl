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
        shared_ptr<Json::Value> searchSongs(const string &filter);
        shared_ptr<Json::Value> loadExplore();
        shared_ptr<Gear::PlaybackData> playDirect(const string &directId);
        shared_ptr<PlaybackData> play(const string &songId);
        shared_ptr<Json::Value> fetchArtist(const string &artistId);
        shared_ptr<Json::Value> fetchAlbum(const string &albumId);
        shared_ptr<Json::Value> fetchRadioFeed(const string &radioId);
        shared_ptr<Json::Value> fetchSharedSongs(const string &token);
        shared_ptr<Json::Value> fetchRatedSongs();
        shared_ptr<Json::Value> addSongsToLocker(const vector<string> &songIds);
        shared_ptr<Json::Value> loadRadio();
        shared_ptr<Json::Value> createStation(const string &songId, const string &name, int type);
        void deleteStation(const string &radioId);
        void deletePlaylist(const string &playlistId);
        void getAllSongs(const function<void(const Json::Value &)> &songs);
        shared_ptr<Json::Value> getListOfPlaylists();
        shared_ptr<Json::Value> getPlaylist(const string &playlistId);
        //shared_ptr<Json::Value> getAllPlaylists();
        shared_ptr<Json::Value> addPlaylistEntries(const vector<string> &songs, const string &playlist);
        shared_ptr<Json::Value> createPlaylist(const vector<string> &songIds, const string &title);
        void deletePlaylistEntries(const vector<string> &entries, const vector<string> &songs, const string &playlist);
        void changePlaylistName(const string &name, const string &playlist);
        void changePlaylistOrder(const string &playlist, const vector<string> &movedSongs, const vector<string> &movedEntries, const string &beforeEntry, const string &afterEntry);
        void changeSongs(const Json::Value &changeDictionaries);
        
    private:
        shared_ptr<GoogleMusicConnection> _connection;
        
        bool _allAccess;
        bool _cancelling;
        
        shared_ptr<Json::Value> call(const string &name, const Json::Value &param);
        shared_ptr<PlaybackData> play(const string &aSongId, const string &paramName);
        static shared_ptr<Json::Value> doGetListOfPlaylists(const std::string &str);
        static shared_ptr<Json::Value> doGetListOfPlaylistsOld(const std::string &str);
    };
}

#endif /* defined(__G_Ear_Player__GoogleMusicProtocol__) */
