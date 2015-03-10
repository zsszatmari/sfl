//
//  GooglePlaySessionImpl.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#include <iostream>
#include "GooglePlaySessionImpl.h"
#include "GooglePlayConceiver.h"
#include "IKeychain.h"
#include "GooglePlayService.h"
#include "GoogleMusicProtocol.h"
#include "PlaybackData.h"
#include "NamedImage.h"
#include "WebCookie.h"
#include "OfflineStorage.h"
#include "BackgroundExecutor.h"
#include "Tracking.h"
#include "IPreferences.h"
#include "GoogleMusicConnection.h"
#include "BaseUtility.h"
#include "entities.h"
#include "sfl/Prelude.h"
#include "sfl/Vector.h"
#include "Db.h"
#include "IoService.h"
#include "IApp.h"

namespace Gear
{
	using namespace Gui;
    using namespace sfl;

#define method GooglePlaySession::
    
    shared_ptr<GooglePlaySession> method create(const shared_ptr<IApp> &app)
    {
        return GooglePlaySessionImpl::create(app);
    }
    
#undef method
    
#define method GooglePlaySessionImpl::
    
    shared_ptr<GooglePlaySessionImpl> method create(const shared_ptr<IApp> &app)
    {
        shared_ptr<GooglePlaySessionImpl> ret(new GooglePlaySessionImpl(app));
        ret->init();
        
        return ret;
    }

    void method init()
    {
        shared_ptr<GooglePlaySessionImpl> ret = shared_from_this();
        
        _protocol = shared_ptr<GoogleMusicProtocol>(new GoogleMusicProtocol(GoogleMusicConnection::create()));
    }
    
    void method connect(const string &user, const string &keychainData, const function<void(bool)> &callback)
    {
        assert(Io::isCurrent());
        
        auto protocol = _protocol;
        auto self = shared_from_this();
        
        bool success = false;
            
        Json::Reader reader;
        Json::Value cookies;
        if (reader.parse(keychainData, cookies)) {
            protocol->setUserName(user);
        }
        success = protocol->loginWithCookies(cookies);   
        if (!success) {
            callback(false);
        } else {       
    
            // must do this here, because we need info on whether all access is present
            checkAllAccess([self,this,protocol,callback]{
                Tracking::track("Google Music", "All Access", allAccessCapable());
                
                vector<WebCookie> newCookies = protocol->cookiesToSave();
                Json::Value cookiesArr;
                
                for (auto itCookie = newCookies.begin() ; itCookie != newCookies.end() ; ++itCookie) {
                    const WebCookie &cookie = *itCookie;

                    Json::Value oneCookie;
                    auto props = cookie.properties();
                    for (auto it = props.begin() ; it != props.end() ; ++it) {
                        auto &p = *it;

                        oneCookie[p.first] = p.second;
                    }
                    cookiesArr.append(oneCookie);
                }
                Json::FastWriter writer;
                auto encodedCookies = writer.write(cookiesArr);
                
                IApp::instance()->keychain()->save(GooglePlayService::staticIdentifier(), protocol->userName(), encodedCookies);
                
                // because pThis is also retained, this is guaranteed to survive
                _connectedSignal.signal();

                callback(true);
            
            });
        }
    }
    
    method GooglePlaySessionImpl(const shared_ptr<IApp> &app) :
            GooglePlaySession(shared_ptr<const IConceiverFactory<GooglePlaySession>>(new ConceiverFactory<GooglePlayConceiver, GooglePlaySession>()), app),
            _cacheFetched(false)
    {
    }
    
    static const vector<string> stringFields()
    {
        string s[] = {"album", "albumArtist", "artist", "genre", "title", "playlistEntryId"};
        return init<vector<string>>(s);
    }
    
    static const vector<string> intFields()
    {
        string i[] = {"creationDate", "lastPlayed", "playCount", "disc", "track", "durationMillis", "totalDiscs", "totalTracks", "year", "rating"};
        return init<vector<string>>(i);
    }
    
    static shared_ptr<Json::Value> sanitize(const shared_ptr<Json::Value> &value)
    {
        if (!value) {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        return value;
    }
    
    void method checkAllAccess(const function<void()> &done)
    {
        auto self = shared_from_this();
        _protocol->loadExplore([self,this,done](const shared_ptr<Json::Value> &ret){
            
            if (ret) {
                auto &json = ret;
                auto count = json->size();
                //std::cout << "explorecount: " << count << std::endl;
                if (count >= 2) {
                    //std::cout << "explorecount: " << count << "/" << (*json)[1].size() << std::endl;
                    
                    setAllAccessCapable(true);
                    playlistsChanged();
                }
            }
            done();
        });
    }
    
    void method searchAllAccess(const string &filter, const function<void(const shared_ptr<Json::Value> &)> &callback)
    {
        _protocol->searchSongs(filter,[callback](const shared_ptr<Json::Value> &value){
            callback(sanitize(value));
        });
    }
    
    shared_ptr<IPlaybackData> method playbackDataSync(const string &uniqueId) const
    {
#pragma message("TODO: play offline downloaded songs locally")
        //if (OfflineStorage::instance().available(uniqueId)) {
        //    return OfflineStorage::instance().fetch(uniqueId, IPlaybackData::Format::Mp3);
        //}
        
        if (isAllAccessId(uniqueId)) {
            
            return _protocol->playDirect(uniqueId);
        } else {
            return _protocol->play(uniqueId);
        }
    }
    
    void method fetchArtist(const string &artistId, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        _protocol->fetchArtist(artistId,[f](const shared_ptr<Json::Value> &value){
            f(sanitize(value));
        });
    }
    
    void method fetchAlbum(const string &albumId, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        _protocol->fetchAlbum(albumId,[f](const shared_ptr<Json::Value> &value){
            f(sanitize(value));
        });
    }
    
    void method addAllAccessIdsToLibrary(const vector<string> &songs, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        _protocol->addSongsToLocker(songs,[f](const shared_ptr<Json::Value> &value){
            f(sanitize(value));
        });
    }
    
    void method fetchRadioFeed(const string &radioId, const function<void(const shared_ptr<Json::Value> &)> &f) const
    {
        _protocol->fetchRadioFeed(radioId,[f](const shared_ptr<Json::Value> &value){
            f(sanitize(value));
        });
    }
    
    void method fetchSharedSongs(const string &playlistId, const function<void(const shared_ptr<Json::Value> &)> &f) const
    {
        _protocol->fetchSharedSongs(playlistId,[f](const shared_ptr<Json::Value> &value){
            f(sanitize(value));
        });
    }

    void method fetchRatedSongs(const function<void(const shared_ptr<Json::Value> &)> &f) const
    {
        _protocol->fetchRatedSongs([f](const shared_ptr<Json::Value> &value){
            f(sanitize(value));
        });
    }
    
    void method loadRadio(const function<void(const shared_ptr<Json::Value> &)> &f) const
    {
        _protocol->loadRadio([f](const shared_ptr<Json::Value> &value){
            f(sanitize(value));
        });
    }
    
    void method createRadio(const string &songId, const string &name, const int type, const function<void(const shared_ptr<Json::Value> &)> &f) const
    {
        _protocol->createStation(songId, name, type,[f](const shared_ptr<Json::Value> &value){
            f(sanitize(value));
        });
    }
    
    void method removeRadio(const string &radioId)
    {
        _protocol->deleteStation(radioId);
    }
    
    void method removePlaylist(const string &playlistId)
    {
        _protocol->deletePlaylist(playlistId);
    }
    
    vector<Json::Value> method conceive(const Json::Value &json, GooglePlayConceiver::Hint hint, const std::string &playlistId)
    {
        auto pThis = shared_from_this();
        auto conceiver = _conceiverFactory->conceiver(json, pThis, static_cast<int>(hint));
        
        if (playlistId.empty()) {
            return vector<Json::Value>();
        }
        
        auto librarySongs = conceiver->songs(playlistId);
        if (conceiver->starRatedPresent()) {
            auto &prefs = IApp::instance()->preferences();
            string autoKey = "FiveStarRatingsAuto";

            if (!prefs.boolForKey(autoKey)) {
                prefs.setUintForKey(autoKey, true);
                prefs.setUintForKey("FiveStarRatings", true);
            }
        }

        return librarySongs;
    }

    void method fetchLibrary(const std::function<bool(const vector<Json::Value> &)> &gotDictionaries, const std::function<void()> &gotAll)
    {
        assert(Io::isCurrent());

        auto pThis = MEMORY_NS::static_pointer_cast<GooglePlaySessionImpl>(shared_from_this());
        auto gotDictionariesRaw = [=](const Json::Value &aSongs){

            if (aSongs.size() < 1) {
                auto ret = gotDictionaries(std::vector<Json::Value>());
#ifdef DEBUG
                std::cout << "library fetched and finalized\n";
#endif
                gotAll();
            }
            return gotDictionaries(conceive(aSongs[0u], GooglePlayConceiver::Hint::loadLibrary, "all"));
        };
        
        _protocol->getAllSongs(gotDictionariesRaw);
    }

    void method fetchPlaylists(const vector<Json::Value> &playlistItems, const function<bool(const string &,const vector<Json::Value> &,bool)> &callback, const function<void()> &signalEnd)
    {
        if (playlistItems.empty()) {
            signalEnd();
        } else {
            auto playlistItem = head(playlistItems);
            auto self = shared_from_this();
            auto cont = [self,this,playlistItems,callback,signalEnd]{
                fetchPlaylists(tail(playlistItems), callback, signalEnd);
            };

            if (playlistItem.size() < 5) {
                cont();
                return;
            }
            string playlistId = playlistItem[0u].asString();        
            string name = decode_html_entities_utf8(playlistItem[1u].asString());
            
            if (playlistId.empty()) {
                // some kind of google db failure
                cont();
                return;
            }
            
            bool isShared = false;
            string token;
            auto type = playlistItem[4u].asInt();
            shared_ptr<Json::Value> pSongs;
            if (type == 5) {
                // shared playlist
                isShared = true;
                int count = playlistItem.size();
                if (count > 5) {
                    token = playlistItem[5u].asString();;
                } else {
                    cont();
                    return;
                }

                Json::Value list;
                list["category"] = "playlist";
                list["dynamic"] = true;
                list["playlistId"] = playlistId;
                list["name"] = name;

                callback("Playlist", Vector::singleton(list), false);
                cont();
            } else {
                #pragma message("TODO: this is somehow not right... requests are running parallel, and eof signal is obfuscated")
                _protocol->getPlaylist(playlistId, [callback,cont,playlistId,name,self,this](const shared_ptr<Json::Value> &pSongs){
                    if (!pSongs || !pSongs->isArray()) {
                        cont();
                        return;
                    }

                    auto conceived = conceive(*pSongs, GooglePlayConceiver::Hint::loadPlaylist, playlistId);
                    int position = 1;
                    for (auto &song : conceived) {
                        song["position"] = position;
                        ++position;
                    }
                    Json::Value list;
                    list["category"] = "playlist";
                    list["name"] = name;
                    list["playlistId"] = playlistId;
                    callback("Playlist", Vector::singleton(list), false);
                    callback("Song", conceived, false);
                    cont();
                });
            }
        }
    }
    
    void method fetchPlaylists(const function<bool(const string &,const vector<Json::Value> &,bool)> &callback, const function<void()> &signalEnd)
    {
        auto self = shared_from_this();
        _protocol->getListOfPlaylists([callback,signalEnd,self,this](const shared_ptr<Json::Value> &response){

            if (!response) {
    #ifdef DEBUG
                std::cout << "playlists: none\n";
    #endif
                signalEnd();
                return;
            }
            size_t size = response->size();
    #ifdef DEBUG
            std::cout << "playlists: " << size << "\n";
    #endif
            vector<Json::Value> playlistItems;
            for (int i = 0 ; i < size ; ++i) {
                playlistItems.push_back((*response)[i]);
            }
            fetchPlaylists(playlistItems, callback, signalEnd);
        });
    }
    
    void method addSongsToPlaylist(const string &playlistId, const vector<string> &songIds, const function<void(const shared_ptr<Json::Value> &)> &callback)
    {
        _protocol->addPlaylistEntries(songIds, playlistId, callback);
    }

    void method createPlaylist(const vector<string> &songIds, const string &name, const function<void(const shared_ptr<Json::Value> &)> &callback)
    {
        _protocol->createPlaylist(songIds, name, [callback](const shared_ptr<Json::Value> &result){
            callback(sanitize(result));
        });
    }
    
    void method deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId)
    {
        if (_protocol) {
            _protocol->deletePlaylistEntries(entryIds, songIds, playlistId);
        }
    }
    
    void method changePlaylistNameSync(const string &newName, const string &playlistId)
    {
        _protocol->changePlaylistName(newName, playlistId);
    }
    
    void method changePlaylistOrderSync(const string &playlistId, const vector<string> &songIds, const vector<string> &entryIds, const string &afterId, const string &beforeId)
    {
        _protocol->changePlaylistOrder(playlistId, songIds, entryIds, beforeId, afterId);
    }
    
    void method modifySongsSync(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts)
    {
        Json::Value dictionaries(Json::arrayValue);
        for (auto it = ids.begin() ; it != ids.end() ; ++it) {
        	auto &songId = *it;

        	Json::Value dictionary(Json::objectValue);
            dictionary["id"] = songId;
            for (auto itp = changedStrings.begin() ; itp != changedStrings.end() ; ++itp) {
            	auto &p = *itp;
            	dictionary[p.first] = p.second;
            }

            for (auto itp = changedInts.begin() ; itp != changedInts.end() ; ++itp) {
            	auto &p = *itp;
            	dictionary[p.first] = p.second;
            }
            dictionaries.append(dictionary);
        }
        
        _protocol->changeSongs(dictionaries);
    }
    
    string method userName() const
    {
        return _protocol->userName();
    }
}
