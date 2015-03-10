//
//  GoogleMusicProtocol.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/9/13.
//
//

#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
//#include <pwd.h>
#include "GoogleMusicProtocol.h"
#include "json.h"
#include "IApp.h"
#include "GooglePlaySessionImpl.h"
#include "HttpDownloader.h"
#include "GooglePlaySignature.h"
#include "PlaybackData.h"
#include "UrlEncode.h"
#include "GoogleMusicConnection.h"
#include "Logger.h"
#include "JsonStreamer.h"

using std::vector;
using std::map;
using std::cout;
using std::endl;
using std::make_pair;

namespace Gear
{    
#define method GoogleMusicProtocol::
    
    method GoogleMusicProtocol(const shared_ptr<GoogleMusicConnection> &connection) :
        _cancelling(false),
        _allAccess(false),
        _connection(connection)
    {
    }
    
    void method setUserName(const string &name)
    {
        _connection->setUserName(name);
    }
    
    const string & method userName() const
    {
        return _connection->userName();
    }
    
    bool method loginWithCookies(const Json::Value &cookiesJson)
    {
        auto success = _connection->loginWithCookies(cookiesJson);
        
        return success;
    }
    
    const vector<Gear::WebCookie> & method cookiesToSave() const
    {
        return _connection->cookiesToSave();
    }
    
    static const Json::Value jsonWithSecond(const Json::Value &second)
    {
        Json::Value first(Json::arrayValue);
        first.append(Json::Value(Json::nullValue));
        first.append(1);
        
        Json::Value parameters(Json::arrayValue);
        parameters.append(first);
        parameters.append(second);
        return parameters;
    }
    
    void method loadExplore(const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        //[["a1jlniabk2ps",1],[[1],null,""]]:
    
        Json::Value second(Json::arrayValue);
        Json::Value sub(Json::arrayValue);
        sub.append(1);
        second.append(sub);
        second.append(Json::nullValue);
        second.append("");
        
        call("explore/loadexplore", jsonWithSecond(second), f);
    }
    
    void method searchSongs(const string &filter, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        // [["mnaeqep1jn91",1],["skinn",10]]:
        
        Json::Value second(Json::arrayValue);
        second.append(filter);
        second.append(10);
        
        call("search", jsonWithSecond(second), f);
    }
    
    
    shared_ptr<Gear::PlaybackData> method playDirect(const string &directId)
    {
        return play(directId, "mjck");
    }
    
    shared_ptr<PlaybackData> method play(const string &songId)
    {
        if (!songId.empty() && songId.find("-") == string::npos) {
            // for playback of playlist-only items
            return playDirect(songId);
        }
        return play(songId, "songid");
    }
    
    
    void method fetchArtist(const string &artistId, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        // [["z9qwq5l8dfbi",1],[["Abrieyefqd7zyfsnd7ttway5qgy"]]]:
        //id parameters = @[@[[NSNull null],@1], @[@[artistId]]];
        
        Json::Value value(Json::arrayValue);
        Json::Value inner(Json::arrayValue);
        inner.append(artistId);
        value.append(inner);
        
        call("fetchartist", jsonWithSecond(value), f);
    }
    
    void method fetchAlbum(const string &albumId, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        // [["z9qwq5l8dfbi",1],[["B6o7ehs7mx4akcgi3uiw4s3nehm"]]]:
        Json::Value value(Json::arrayValue);
        Json::Value inner(Json::arrayValue);
        inner.append(albumId);
        value.append(inner);
        
        call("fetchalbum", jsonWithSecond(value), f);
    }
    
    void method fetchRadioFeed(const string &radioId,  const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        // i'm feeling lucky radio vs normal radio
        //[["u6w14lj0hxu6",1],["09afada4-133a-3fea-b144-3431f2c1a66d",[],null,null,false,[]]]:
        // [["u6w14lj0hxu6",1],[null,[],null,null,false,[[5,""]]]]:
        
        
        Json::Value value(Json::arrayValue);
        bool lucky = false;
        if (!radioId.empty()) {
            value.append(radioId);
        } else {
            // i'm feeling lucky
            value.append(Json::nullValue);
            lucky = true;
        }
        value.append(Json::arrayValue);
        value.append(Json::nullValue);
        value.append(Json::nullValue);
        value.append(false);
        if (lucky) {
            Json::Value outerArray;
            Json::Value innerArray;
            innerArray.append(5);
            innerArray.append("");
            outerArray.append(innerArray);
            value.append(outerArray);
        } else {
            value.append(Json::arrayValue);
        }
        
        call("radio/fetchradiofeed", jsonWithSecond(value), f);
    }
    
    void method fetchSharedSongs(const string &token, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        // [["1bxnpxtf26pc",1],["AMaBXymdk5FNowrxzbw64m76477rlyhBAr_W0N0aUIyrwO8c3LEmVoil7WyaKT61Qr4zC07-B8bkIP_vv9fP2vZQFoDylLC8LA=="]]
        
        Json::Value value(Json::arrayValue);
        value.append(token);
        
        call("loadsharedplaylist", jsonWithSecond(value), f);
    }
    
    void method fetchRatedSongs(const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        // [["l4u6892ukf1k",1],[]]:
        Json::Value value(Json::arrayValue);
        
        call("getephemthumbsup", jsonWithSecond(value), f);
    }

    void method addSongsToLocker(const vector<string> &songIds, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        // [["nw33sz5xm26k",1],[["Trqzaxpzmnuktxu4ufeplvisq2a","Tchvxta3cdhstdkevv6jru7dimi","T7lizw5qh4gmeuubmzcmk3va6pq"]]]:
        Json::Value value(Json::arrayValue);
        Json::Value inner(Json::arrayValue);
        for (auto it = songIds.begin() ; it != songIds.end() ; ++it) {
            inner.append(*it);
        }
        value.append(inner);
        
        return call("addsongstolocker", jsonWithSecond(value), f);
    }
    
    void method loadRadio(const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        // [["serkl9qvxr7",1],[null,false]]:
        Json::Value value(Json::arrayValue);
        value.append(Json::nullValue);
        value.append(false);
        
        call("radio/loadradio" ,jsonWithSecond(value), f);
    }
    
    void method createStation(const string &songId, const string &name, int type, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        //[["b54u37kejppp",1],["50ead1ff-7a1f-3017-98cc-6a421c8428a7",1,"Baltabolt",null,false]]:
        
        /*  type: 0: normal song
         1: allaccess 'direct'
         2: album
         3: artist
         */
        
        Json::Value value(Json::arrayValue);
        value.append(songId);
        value.append(type);
        value.append(name);
        value.append(Json::nullValue);
        value.append(false);
        
        call("radio/createstation", jsonWithSecond(value), f);
    }
    
    void method deleteStation(const string &radioId)
    {
        // [["4fyd8rabqlh",1],["91bf31be-b2ad-3beb-a896-38ab5d146489"]]:
        
        Json::Value value(Json::arrayValue);
        value.append(radioId);
        
        call("radio/deletestation", jsonWithSecond(value), [](const shared_ptr<Json::Value> &){});
    }
    
    void method deletePlaylist(const string &playlistId)
    {
        //deleteplaylist
        //json:{"id":"88ad55b1-0f38-4d1f-ad63-f3dc3c60a421","requestCause":1,"requestType":1,"sessionId":"zaz9rcgkp6ds"}
        
        Json::Value parameters(Json::objectValue);
        parameters["id"] = playlistId;
        parameters["requestCause"] = 1;
        parameters["requestType"] = 1;
        
        call("deleteplaylist", parameters, [](const shared_ptr<Json::Value> &ret){

            if (!ret) {
                cout << "error deleting playlist" << endl;
            }
        });
    }
        
    void method getAllSongs(const function<bool(const Json::Value &)> &songRetrieveBlock)
    {
        // initial call
        _cancelling = false;
        
        
        Json::Value params(Json::objectValue);
        // "{"tier":2,"requestCause":1,"requestType":1,"sessionId":"mjug4afewv22"}"
        params["tier"] = 2;
        params["requestCause"] = 1;
        params["requestType"] = 1;
        params["sessionId"] = Json::nullValue;
        
        _connection->streamingCall("streamingloadalltracks", params, [songRetrieveBlock](const Json::Value &chunk){
            if (chunk.empty()) {
                // we shall see if this is really needed
                //_cancelling = false;
            }
            songRetrieveBlock(chunk);
        });
        /*for (;;) {
            Json::Value chunk;
            bool hasMore = streamer->nextChunk(chunk);
            
            if (!songRetrieveBlock(chunk)) {
                _cancelling = true;
            }
            if (!hasMore) {
                break;
            }
            
            if (_cancelling) {
                songRetrieveBlock(Json::Value());
                return;
            }
        }*/
    }
    
    void method getListOfPlaylists(const function<void(const shared_ptr<Json::Value> &)> &callback) 
    {
        _connection->callPure("listen",[callback](const string &str){
            /*struct passwd *pw = getpwuid(getuid());
            auto filename = std::string(pw->pw_dir) + "/Desktop/geardebugplaylistfetch.txt";
            std::ofstream out(filename.c_str());
            out << str;
            out.close();
            */
            
            auto ret = doGetListOfPlaylists(str);
            if (!ret) {
                callback(doGetListOfPlaylistsOld(str));
            }
            callback(ret);
        });
    }

    static void replaceOccurrences(string &str, const string &search, const string &replace)
    {
        size_t pos = 0;
        while ((pos = str.find(search,pos)) != std::string::npos) {
            str.replace(pos, search.length(), replace);
        }
    }

    static shared_ptr<Json::Value> processPlaylists(const std::string &content, int level)
    {
        Json::Reader reader;
        Json::Value json;
        reader.parse(content, json);

        while(level > 0) {
            if (!json.isArray()) {
                return shared_ptr<Json::Value>();
            }
            //std::cout << "array level " << level << " count: " << json.size() << std::endl;

            --level;
            bool found = false;
            for (int i = 0 ; i < json.size() ; ++i) {
                auto &sub = json[i];
                if (sub.isArray()) {
                    // first item which is an array will be the list of playlists 
                    found = true;
                    json = sub;
                    break;
                }
            }
            if (!found) {
                return shared_ptr<Json::Value>();
            }
        }
        //std::cout << "array level " << level << " count: " << json.size() << std::endl;

        if (json.isArray()) {
            return shared_ptr<Json::Value>(new Json::Value(json));
        } else {
            return shared_ptr<Json::Value>();
        }
    }

    shared_ptr<Json::Value> method doGetListOfPlaylists(const std::string &str)
    {
        const string kBegin = "SJ_initialize(";
        size_t beginpos = str.find(kBegin);
        if (beginpos == string::npos) {
            return shared_ptr<Json::Value>();
        }
        beginpos += kBegin.length();

        const string kEnd = ");";
        size_t endpos = str.find(kEnd, beginpos);
        if (endpos == string::npos) {
            return shared_ptr<Json::Value>();
        }
        string content = str.substr(beginpos,endpos-beginpos);
        replaceOccurrences(content, "&quot;", "\"");
        GoogleMusicConnection::fixJson(content);

        //content = UrlEncode::decode(content);
#if DEBUG
        //std::cout << "found: " << content << std::endl;
#endif

        return processPlaylists(content, 1);
    }

    shared_ptr<Json::Value> method doGetListOfPlaylistsOld(const std::string &str)
    {
        const string kBegin = "window['USER_CONTEXT'] =";
        size_t beginpos = str.find(kBegin);
        if (beginpos == string::npos) {
            return shared_ptr<Json::Value>();
        }
        beginpos += kBegin.length();
        
        string content = str.substr(beginpos);
        GoogleMusicConnection::fixJson(content);
        
        return processPlaylists(content, 1);
    }

    void method getPlaylist(const string &playlistId, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        // [["yra5o0x0ibu7",1],["80125b73-ebe6-4f34-ab7a-c8ea27b74e7d"]]:
        Json::Value value(Json::arrayValue);
        value.append(playlistId);
        
        call("loaduserplaylist", jsonWithSecond(value), f);
    }
    
    void method addPlaylistEntries(const vector<string> &songs, const string &playlist, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        /*
         addtoplaylist
         json:{"playlistId":"9c0f7728-5c52-42bc-bc4f-3d132bd87c3d","songRefs":[{"id":"40cfbf54-32e5-36bf-a916-91f18211fbd0","type":1}],"sessionId":"4x9f3hxid3ht"}
         
         response:
         {"playlistId":"9c0f7728-5c52-42bc-bc4f-3d132bd87c3d","songIds":[{"playlistEntryId":"ccbc73a8-8eb2-3302-82eb-7d8d29c62ede","songId":"40cfbf54-32e5-36bf-a916-91f18211fbd0"}]}
         */
        
        Json::Value songRefs(Json::arrayValue);
        for (auto it = songs.begin() ; it != songs.end() ; ++it) {
        	auto &songId = *it;

            int type;
            if (songId.find("-") == string::npos) {
                // direct
                type = 2;
            } else {
                // library
                type = 1;
            }
            
            Json::Value value(Json::objectValue);
            value["id"] = songId;
            value["type"] = type;
            songRefs.append(value);
        }
        
        Json::Value parameters(Json::objectValue);
        parameters["playlistId"] = playlist;
        parameters["songRefs"] = songRefs;
        parameters["sessionId"] = Json::nullValue;
        
        call("addtoplaylist", parameters,[parameters,f](const shared_ptr<Json::Value> &ret){

#define DEBUG_ADDPLAYLISTENTRIES
#ifdef DEBUG_ADDPLAYLISTENTRIES
            Json::FastWriter writer;
            Logger::stream() << "add to playlist body: " << writer.write(parameters) << "\n";
            if (!ret) {
                Logger::stream() << "add to playlist null reply\n";
            } else {
                Logger::stream() << "add to playlist reply: " << writer.write(*ret) << "\n";
            }
#endif
            f(ret);
        });
    }
    
    void method createPlaylist(const vector<string> &songIds, const string &title, const function<void(const shared_ptr<Json::Value> &)> &f)
    {
        //[["i61nrvodvruv",1],[false,"June 14, 2013",null,[["c2bc4691-9ef0-320e-aa3a-b4bcbd2d249f",6],["32f384f2-923b-32c4-a291-df8666f77e4f",7],["50ead1ff-7a1f-3017-98cc-6a421c8428a7",7]]]]:
        
        // don't know what the hell '5', '6' or '7' could be!? maybe 5: all access, 6: library, 7: allaccessbutinlibrary
        
        Json::Value songRefs(Json::arrayValue);
        for (auto it = songIds.begin() ; it != songIds.end() ; ++it) {
        	const string &song = *it;

        	int type = 5;
            if (song.find("-") != string::npos) {
                type = 7;
            }
            Json::Value value(Json::arrayValue);
            value.append(song);
            value.append(type);
            songRefs.append(value);
        }
        
        Json::Value value(Json::arrayValue);
        value.append(0);
        value.append(title);
        value.append(Json::nullValue);
        value.append(songRefs);
        
        call("createplaylist", jsonWithSecond(value), f);
    }
    
    static Json::Value jsonArray(const vector<string> &strings)
    {
        Json::Value value(Json::arrayValue);
        for (auto it = strings.begin() ; it != strings.end() ; ++it) {
        	auto &str = *it;

        	value.append(str);
        }
        return value;
    }
    
    void method deletePlaylistEntries(const vector<string> &entries, const vector<string> &songs, const string &playlist)
    {
        // music/services/deletesong
        //json:{"songIds":["75552256-ecca-370b-a327-6c782bf1935a"],"entryIds":["babee2be-ae0c-3586-9332-4f018e52e1cc"],"listId":"db563d00-bc92-3d4b-9b41-bdd75de0105c","sessionId":"nscdchdle4dy"}
        
        Json::Value parameters(Json::objectValue);
        Json::Value songsValue = jsonArray(songs);
        Json::Value entriesValue = jsonArray(entries);
        
        parameters["songIds"] = songsValue;
        parameters["entryIds"] = entriesValue;
        parameters["listId"] = playlist;
        
        call("deletesong", parameters, [](const shared_ptr<Json::Value> &){});
    }

    void method changePlaylistName(const string &name, const string &playlist)
    {
        // music/services/editplaylist
        
        // [["r8kedtpp7vxj",1],["9314fde9-9622-4481-b577-23009fc95962",false,"Hieroglyphttt",""]]:
        
        Json::Value value(Json::arrayValue);
        value.append(playlist);
        value.append(false);
        value.append(name);
        value.append("");
        
        call("editplaylist", jsonWithSecond(value), [](const shared_ptr<Json::Value> &){});
    }
    
    void method changePlaylistOrder(const string &playlist, const vector<string> &movedSongs, const vector<string> &movedEntries, const string &beforeEntry, const string &afterEntry)
    {
        // music/services/changeplaylistorder
        // json:{"playlistId":"db563d00-bc92-3d4b-9b41-bdd75de0105c","movedSongIds":["91892cb9-5eae-3003-8cc2-e3d3719d28ac"],"movedEntryIds":["2f7b4304-f111-3933-91a1-9be25a21c0cd"],"afterEntryId":"6e057e5f-a14d-3fa9-8bf0-b5de2ea95f4e","beforeEntryId":"4fa4363b-e178-323d-a7b2-d3a2b4676a2b","sessionId":"remgviosxcp0"}
        
        Json::Value parameters(Json::objectValue);
        parameters["playlistId"] = playlist;
        parameters["movedSongIds"] = jsonArray(movedSongs);
        parameters["movedEntryIds"] = jsonArray(movedEntries);
        parameters["afterEntryId"] = afterEntry;
        parameters["beforeEntryId"] = beforeEntry;
        
        call("changeplaylistorder", parameters, [](const shared_ptr<Json::Value> &ret){
            if (!ret) {
                cout << "error modifying playlist name" << endl;
            }
        });
    }
    
    void method changeSongs(const Json::Value &changeDictionaries)
    {
        // this worked in the past:
        // json=%7B%22entries%22%3A%5B%7B%22id%22%3A%22f67c0828-ec02-363d-b98f-fa14cfc81cb1%22%2C%22rating%22%3A5%7D%5D%2C%22sessionId%22%3A%22d0829evq6jq7%22%7D
        
        // but now we need something like this:
        // services/modifytracks
        // title change: [["56367ks4japd",1],[[["89ccc785-a25a-3b1f-81bc-6e35bac29cf2","04 - Remember a day.mp33",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,[],null,null,true]]]]:
        // rating change: [["56367ks4japd",1],[[["4969182d-3d66-3baa-8144-fb892f38e2e0",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,5,null,null,null,null,null,2,null,null,null,null,null,null,null,null,null,[]]]]]:
        // multiple songs: [["ay1yqsczb9sc",1],[[["e8c91f23-44b1-3f35-9be8-64e5379d4c45",null,null,"Pink Floydd",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,[]],["b3019b9c-2fdf-35cb-b710-72b316e7f7b8",null,null,"Pink Floydd",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,[]]]]]:
        
        std::vector<string> recordPlayback;
        
        Json::Value transformed(Json::arrayValue);
        for (auto it = changeDictionaries.begin() ; it != changeDictionaries.end() ; ++it) {
        	Json::Value dict = *it;

        	if (!dict.isObject()) {
                continue;
            }
            
            Json::Value song;
            if (dict["id"].asString().find("-") == string::npos) {
                // type: it is an all access song
                song[29] = 5;
            }
            
            song[0] = dict["id"];
            static const auto keysToIndexes = GooglePlayConceiver::keysToIndexesTable();
            
            bool modified = false;
            for (const auto &key : dict.getMemberNames()) {
                auto itIndex = keysToIndexes.find(key);
                if (key == "playCount") {
                    recordPlayback.push_back(dict["id"].asString());
                }
                if (itIndex == keysToIndexes.end()) {
                    continue;
                }
                modified = true;
                song[itIndex->second] = dict[key];
            }
            
            if (modified) {
                transformed.append(song);
            }
        }
        
        if (transformed.size() > 0) {
            Json::Value container;
            container.append(transformed);
            
            call("modifytracks", jsonWithSecond(container),[](const shared_ptr<Json::Value> &ret){
                if (!ret) {
                    cout << "error updating song!" << endl;
                }
            });
        }
        
        if (!recordPlayback.empty()) {
            for (auto &songId : recordPlayback) {
                Json::Value value(Json::objectValue);
                //	json:{"songId":"c002a525-7900-30c6-b0d3-10d6bd13c7ee","updateRecentAlbum":false,"updateRecentPlaylist":false,"playlistId":"","type":2,"playbackContext":1,"playbackType":2,"sessionId":"ay1yqsczb9sc"}
                //	json:{"songId":"Thzdii36ryaiit7bacj4b2k62im","updateRecentAlbum":false,"updateRecentPlaylist":false,"playlistId":"","type":5,"playbackContext":1,"playbackType":2,"sessionId":"ay1yqsczb9sc"}
                //	json:{"songId":"6c1f1855-3698-32fe-9405-d19db4c80a6c","updateRecentAlbum":false,"updateRecentPlaylist":true,"playlistId":"deb886eb-dc85-4072-8d90-4cd9b29cd0cb","type":7,"playbackContext":4,"playbackType":2,"sessionId":"ay1yqsczb9sc"}
                
                value["songId"] = songId;
                value["updateRecentAlbum"] = false;
                value["updateRecentPlaylist"] = false;
                value["playlistId"] = "";
                int type;
                if (songId.find("-") == string::npos) {
                    // direct
                    type = 5;
                } else {
                    // library
                    type = 2;
                }
                value["type"] = type;
                value["playbackContext"] = 1;
                value["playbackType"] = 2;
                //value["sessionId"] = Json::nullValue;
                
                call("recordplaying", value, [](const shared_ptr<Json::Value> &){});
            }
        }
    }
    
    void method call(const string &name, const Json::Value &param, const function<void(const shared_ptr<Json::Value> &)> &callback)
    {
        _connection->call(name, param, callback);
    }

    shared_ptr<PlaybackData> method play(const string &aSongId, const string &paramName)
    {
        GooglePlaySignature signature;
        
        //NSMutableURLRequest *request = [self buildRequest:[NSString stringWithFormat:@"https://music.google.com/music/play?u=0&pt=e&%@=%@", paramName, songId]];
        
        std::stringstream str;
        str << "https://play.google.com/music/play?u=0&pt=e&" << paramName << "=" << aSongId << "&slt=" << signature.salt() << "&sig=" << signature.generateForSongId(aSongId);
        const string url = str.str();
        
//#if DEBUG
        Logger::stream() << "send play request: " << url << "\n";
//#endif
        
        auto headers = _connection->buildHeaders();
        
        auto downloader = HttpDownloader::create(url, headers, "GET");
        downloader->waitUntilFinished();
        
        bool success = !downloader->failed();
        if (success) {
            
            Json::Value json;
            const string data = *downloader;
            
            {
                Json::Reader reader;
                success = reader.parse(data, json);
            };
            
            if (success && json.isObject()) {
                
                const string gotUrl = json["url"].asString();
                if (!gotUrl.empty()) {
                    Logger::stream() << "Got url: " << gotUrl << "\n";
                    vector<string> urls;
                    urls.push_back(gotUrl);
                    return shared_ptr<PlaybackData>(new PlaybackData(urls, IPlaybackData::Format::Mp3));
                }
                
                const Json::Value gotUrls = json["urls"];
                if (gotUrls.isArray() && gotUrls.size() > 0) {
                    Logger::stream() << "Got urls: " << gotUrls.size() << "\n";
                    vector<string> cppUrls;
                    cppUrls.reserve(gotUrls.size());
                    for (auto it = gotUrls.begin() ; it != gotUrls.end() ; ++it) {
                    	auto iUrl = *it;
                        cppUrls.push_back(iUrl.asString());
                    }
                    return shared_ptr<PlaybackData>(new PlaybackData(cppUrls, IPlaybackData::Format::Mp3));
                }
            };
            
            Logger::stream() << "no meaningful data\n";
        } else {
            Logger::stream() << "no data\n";
        }
        return shared_ptr<PlaybackData>();
    }
}

