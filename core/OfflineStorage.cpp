//
//  OfflineStorage.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 29/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "OfflineStorage.h"
#include "IPlaybackData.h"
#include "IApp.h"
#include "IFileManager.h"
#include "FilePlaybackData.h"
#include "OfflineState.h"
#include "json.h"
#include "UrlEncode.h"
#include "IPlaylist.h"
#include "Encoder.h"
#include "IPreferences.h"
#include "IoService.h"
#include "sfl/Prelude.h"
#include "sfl/Maybe.h"
#include "Db.h"
#include "ISession.h"

namespace Gear
{
    class GooglePlaySession;
    
#define method OfflineStorage::
    
    using std::string;
    using std::ofstream;
    using std::ios;
    using THREAD_NS::lock_guard;
    using THREAD_NS::mutex;
    using namespace sfl;

    method OfflineStorage()
    {
    }

    static bool hasEnding (std::string const &fullString, std::string const &ending)
    {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    static string legacyDataPath()
    {
        auto str = IApp::instance()->preferences().stringForKey("OfflineDownloadLocation");
        if (str.empty()) {
            return IApp::instance()->dataPath();
        }
        return str;
    }

    static string legacyPrefix()
    {
        return legacyDataPath() + "/offline-";
    }
    
    static string legacyPlaylistPrefix()
    {
        return legacyDataPath() + "/offlineplaylist-";
    }

    static vector<std::pair<string,bool>> legacyIdentifiersAndEncoded()
    {
        // does not contain prefixes
        auto filenames = IApp::instance()->fileManager()->listFiles(legacyPrefix());
        sort(filenames.begin(), filenames.end());
        vector<std::pair<string,bool>> identifiers;
        identifiers.reserve(filenames.size()/2);

        for (const string &datafileName : filenames) {
            if (hasEnding(datafileName, ".meta")) {
                continue;
            }
            bool encoded = hasEnding(datafileName, ".dat");
            std::string str = datafileName;
            if (encoded) {
                str.erase(str.length() - std::string(".dat").length());
            }

            string meta = str + ".meta";
            auto it = lower_bound(filenames.begin(), filenames.end(), meta);
            if (it == filenames.end() || *it != meta) {
                // delete file
                IApp::instance()->fileManager()->deleteFile(legacyPrefix() + datafileName);
                continue;
            }
            identifiers.push_back(std::make_pair(str, encoded));
        }
        
        return std::move(identifiers);
    }

    static std::string getLegacyFilename(const string &identifier, bool encoded)
    {
        return legacyPrefix() + identifier + (encoded ? ".dat" : "");
    }

    static std::string getLegacyFilename(const string &identifier)
    {
#ifdef __APPLE__
        string encodedName = getLegacyFilename(identifier, true);
        string nonencodedName = getLegacyFilename(identifier, false);
        if (access(encodedName.c_str(), F_OK) != -1) {
            return encodedName;
        }
        if (access(nonencodedName.c_str(), F_OK) != -1) {
            return nonencodedName;
        }
#endif
        return "?";
    }

    static std::string generateFilename(const string &identifier, bool encoded)
    {
        // for now.
        return getLegacyFilename(identifier, encoded);
    }

    static Maybe<Json::Value> valuesForLegacySongIdentifier(const string &identifier, const string &playlistId)
    {
        std::ifstream f((legacyPrefix() + identifier + ".meta").c_str(), ios::in);
        std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();
        if (!f.good()) {
            return Nothing();
        }
        
        Json::Value json;
        Json::Reader reader;
        if (!reader.parse(str, json) || !json.isObject()) {
            return Nothing();
        }
        json["source"] = "gplay";
        json["playlist"] = playlistId;
        json["offlinePath"] = getLegacyFilename(identifier);
        json["offlineRatio"] = 1.0f;

        return json;
    }

    static vector<Json::Value> legacySongs()
    {
        #pragma message("TODO: make sure it plays if encoded, and if not")
        #pragma message("TODO: test that we still have legacy songs and playlists!")
        // bool encoded = hasEnding(datafileName, ".dat");

        return catMaybes(map([](const std::pair<string,bool> &p){return valuesForLegacySongIdentifier(p.first,"all");}, legacyIdentifiersAndEncoded()));
    }

    static std::vector<std::string> playlistIds()
    {
        auto filenames = IApp::instance()->fileManager()->listFiles(legacyPlaylistPrefix());
        return filenames;
    }

    static vector<string> songIdsForPlaylistId(const string &playlistId, string &name)
    {
        auto path = legacyPlaylistPrefix() + playlistId;
        std::ifstream f(path.c_str(), ios::in);
        getline(f, name);
        if (name.empty()) {
            return vector<string>();
        }
        
        std::vector<std::string> songIds;
        while (!f.eof() && !f.fail()) {
            string songId;
            getline(f, songId);
            songIds.push_back(songId);
        }
        return songIds;
    }
    
    static vector<string> songIdsForPlaylistId(const string &playlistId)
    {
        string dummy;
        return songIdsForPlaylistId(playlistId, dummy);
    }

    static std::pair<vector<Json::Value>,vector<Json::Value>> legacyPlaylistsAndTheirSongs()
    {
        auto filenames = playlistIds();
        vector<Json::Value> playlists;
        vector<Json::Value> songs;

        for (auto &ident : filenames) {
            
            std::string name;
            auto songIds = songIdsForPlaylistId(ident, name);
            if (name.empty() || songIds.empty()) {
                continue;
            }
            Json::Value playlist;
            playlist["source"] = "gplay";
            playlist["playlistId"] = ident;
            playlist["category"] = "playlist";
            playlist["name"] = name;
            playlists.push_back(playlist);

            for (auto &songId : songIds) {
                
                auto values = valuesForLegacySongIdentifier(songId, ident);
                match<void>(values,[](const Nothing &){},
                             [&](const Json::Value &json){songs.push_back(json);});
            }
        }
        return std::make_pair(std::move(playlists),std::move(songs));
    }

    void method putLegacyInDb(const shared_ptr<Db> &db)
    {
        Io::get().dispatch([=]{
            assert(Io::isCurrent());

            auto songs = legacySongs();
            auto playlistsAndSongs = legacyPlaylistsAndTheirSongs();

            songs = songs + playlistsAndSongs.second;
            const auto &playlists = playlistsAndSongs.second;

            for (const auto &item : songs) {
                db->upsert("Song",item);
            }
            for (const auto &item : playlists) {
                db->upsert("Playlist",item);
            }
        });
    }

    OfflineStorage & method instance()
    {
        static OfflineStorage inst;
        return inst;
    }
    
    void method update(const shared_ptr<ISong> &song, Maybe<float> ratio)
    {
        
        match<void>(ratio, [=](const Nothing &){
            
            std::cout << "updating " << song->uniqueId() << " to Nothing" << std::endl;
            
            song->setFloatForKey("offlineRatio", 0);
            song->setStringForKey("offlinePath", "");
        }, 
        [=](const float &r){
            std::cout << "updating " << song->uniqueId() << " to " << r << std::endl;
            
            song->setFloatForKey("offlineRatio", r);
        });
        song->updateInDb({"offlineRatio","offlinePath"});

        auto identifier = song->uniqueId();

        auto it = _itemStates.find(identifier);
        if (it != _itemStates.end()) {
            auto s = it->second.lock();
            if (s) {
                match<void>(ratio, [=](const Nothing &){
                    s->setOffline(false, 0);
                }, 
                [=](const float &r){
                    s->setOffline(true, r);
                });
            } else {
                _itemStates.erase(it);
            }       
        }
        
        //if (needUpdate) {
            //_updatedEvent.signal();
        //}
    }
    
    Base::EventConnector method updatedEvent()
    {
        // probably not needed anymore
        return _updatedEvent.connector();
    }

    void method doStore(const string &identifier, const shared_ptr<ISong> &song, const shared_ptr<IPlaybackData> &data, const function<void(bool)> &result)
    {
        assert(Io::isCurrent());

        //Recd::Encoder::generateBinaryKeys(2048, 69);
        string filename = generateFilename(identifier, true);
        song->setStringForKey("offlinePath", filename);
        song->updateInDb({"offlinePath"});
        
        _executor.addTask([=]{
        // prevent creating file if there is a problem receiving data
            data->waitSync(0);
            if (data->failed()) {
                //std::cout << "total failed\n";
                result(false);
                return;
            }
            
            {
                ofstream f(filename.c_str(), ios::out | ios::trunc | ios::binary);
                if (!f.is_open()) {
                    //std::cout << "total not open\n";
                    result(false);
                    return;
                }

                // find out size
                auto length = data->totalLength();
                if (length == 0) {
                    result(false);
                    return;
                }
                //std::cout << "total length: " << length << std::endl;

                int prevPos = 0;
                int pos = 0;

                std::vector<char> encoded;
                auto write = [&]{
                    while (prevPos < pos) {
                        
                        data->accessChunk(prevPos, [&](const char *ptr, int available){
                            
                            // totalLength is not always the truth, sometimes there is extra ~4 bytes
                            if (available == 0 && data->finished()) {
                                pos = prevPos;
                                return;
                            }
                            auto size = std::min<size_t>(available, pos-prevPos);

                            encode(ptr, size, prevPos, encoded);
                            f.write(encoded.data(), encoded.size());
                            prevPos += size;
                        });
                        
                        //std::cout << "downloading... " << prevPos << "/" << length << "  real position: " << (int)f.tellp() << std::endl;
                    }
                    
                    Io::get().dispatch([=]{
                        assert(Io::isCurrent());
                        update(song, ((float)pos)/length);
                    });
                };
                
                for (long long i = 1 ; i <= 99 ; i++) {
                    pos = i * length /100;
                    //std::cout << "downloading " << i << "% bytes: " << pos << std::endl;
                    
                    data->waitSync(pos);
                    if (data->failed()) {
                        result(false);
                        return;
                    }
                    write();
                }
                //data->waitUntilFinished(length-1);
                pos = length;
                write();

                f.close();
                if (!f.good()) {
                    result(false);
                    return;
                }
            }
            result(true);;
        });
    }

    static Maybe<Json::Value> refetch(const shared_ptr<ISong> &song)
    {
        auto pred = [&](const string &key){return ClientDb::Predicate(key, song->stringForKey(key));};
        auto predicate = ClientDb::Predicate(ClientDb::Predicate::Operator::And, {
            pred("source"),pred("playlist"),pred("id")
        });
        auto results = IApp::instance()->db()->fetchSync("Song", predicate, ClientDb::SortDescriptor(), 1, 0, true);
        /*std::cout << "refetch predicate: " << (std::string)predicate << " count: " << results.size() << std::endl;
        if (results.size() > 0) {
            Json::FastWriter writer;
            std::cout << "result: " << writer.write(results.at(0)) << std::endl;
        }*/
        return rangeToMaybe(results);
    }

    static bool shouldStillDownload(const shared_ptr<ISong> &song)
    {
        return match<bool>(refetch(song),[](const Nothing &){return false;},
                                         [](const Json::Value &value){return !value["offlinePath"].asString().empty();});
    }
    
    void method store(const string &identifier, const shared_ptr<ISong> &song)
    {
        assert(Io::isCurrent());

        if (available(*song)) {
            return;
        }
        
        // wait a little, we don't have a .meta file yet, so the gui would be misguided with this update
        //update(identifier, 0.0f);
        
        Json::Value values;
        static std::string ss[] = {"album", "albumArtist", "artist", "genre", "title", "playlistEntryId","creationDate", "lastPlayed", "playCount", "disc", "track", "durationMillis", "totalDiscs", "totalTracks", "year", "rating"};
        for (auto &s : ss) {
            std::string value = song->stringForKey(s);
            values[s] = Json::Value(value);
        }

        song->setStringForKey("offlinePath", ".");
        update(song, 0);

        //std::cout << "store: " << identifier << std::endl;
        
        assert(Io::isCurrent());

            //std::cout << "dostore: " << identifier << std::endl;
            
        auto session = song->session();
        assert(session);

        session->playbackData(*song, [=](const shared_ptr<IPlaybackData> &data){
            assert(Io::isCurrent());

            if (!shouldStillDownload(song)) {
                return;
            }
            auto retryLater = [=]{
                if (!shouldStillDownload(song)) {
                    return;
                } else {
                    update(song, 0);

                    store(identifier,song);
                }
            };

            if (!data) {
                // couldn't fetch data due to error
                //remove(identifier);
                retryLater();
                return;
            }
            
            doStore(identifier, song, data, [=](bool success){
                if (!success) {

                    // don't remove! instead, retry as many times as necessary
                    //remove(identifier);
                    Io::get().dispatch([=]{
                        retryLater();
                    });
                } else {
                    // refresh already existing playlist if necessary
                    //_updatedEvent.signal();
                }
            });
        });
    }

    void method remove(const std::string &identifier, const shared_ptr<ISong> &song)
    {
        auto path = song->stringForKey("offlinePath");
        if (path.length() > 1) {
            IApp::instance()->fileManager()->deleteFile(path);
        }

        update(song, Nothing());
    }

    shared_ptr<OfflineState> method state(const ISong &song)
    {
        auto identifier = song.uniqueId();

        lock_guard<mutex> l(_itemsMutex);
        for (auto it = _itemStates.begin() ; it != _itemStates.end();) {
            if (it->first == identifier) {
                auto s = it->second.lock();
                if (s) {
                    return s;
                } else {
                    //it = _itemStates.erase(it);
                    _itemStates.erase(it++);
                }
            } else {
                if (it->second.expired()) {
                    //it = _itemStates.erase(it);
                    _itemStates.erase(it++);
                } else {
                    ++it;
                }
            }
        }
        
        shared_ptr<OfflineState> ret(new OfflineState());
        ret->setOffline(!song.stringForKey("offlinePath").empty(), song.floatForKey("offlineRatio"));
        auto p = make_pair(identifier, (weak_ptr<OfflineState>)ret);
        _itemStates.insert(p);
        //_itemStates[identifier] = ret;
        return ret;
    }
           
    bool method available(const ISong &song) const
    {
        return song.stringForKey("offlinePath").length() > 1 && song.floatForKey("offlineRatio") > 0.9999;
    }


    static unsigned char kKey[256] = {0x82,0x94,0x72,0x83,0xd7,0x2a,0x31,0x7a,0x95,0x6a,0x5b,0xdc,0xc5,0x43,0x16,0xd8,0x26,0x6a,0xcb,0x4a,0x18,0x8d,0x31,0x59,0xd5,0xdf,0xc4,0x52,0x42,0x5e,0x70,0xd0,0x7,0xf9,0x51,0x9d,0xac,0x58,0xbd,0xc4,0x52,0x91,0xff,0x70,0xe0,0xf9,0x2,0x85,0xf9,0x41,0xc7,0xcb,0xb0,0xeb,0x77,0x12,0xa,0x5d,0x4,0x82,0x61,0x92,0xb6,0x27,0xe,0x96,0xab,0xc0,0x3a,0x9f,0xd0,0xee,0x6a,0x5b,0xdb,0xb8,0xfe,0xf4,0xe9,0xdc,0x28,0x3c,0x25,0x67,0x27,0x48,0x81,0x2f,0xde,0xf9,0x2,0xbc,0x64,0x35,0x6a,0x98,0x2b,0xee,0xd7,0xf3,0x8b,0x41,0x7b,0x6f,0x3,0xdd,0x86,0x98,0x46,0x60,0xb4,0x7e,0x4d,0x75,0x54,0xc6,0x7a,0xde,0xd8,0xa2,0x51,0xe7,0x99,0x51,0xbc,0x3b,0xf3,0xfe,0x11,0xf1,0xd5,0x31,0x15,0x4e,0x32,0xd,0x93,0xd2,0xd2,0x59,0x8c,0x53,0xae,0xb8,0x56,0x73,0x67,0x50,0x32,0x5b,0x3f,0x4e,0x6b,0xae,0x36,0xb6,0xf8,0x6e,0xe,0xd3,0x28,0x21,0x93,0x45,0x17,0x88,0xe8,0x41,0xdb,0xf1,0x3,0xc5,0x37,0x13,0xc6,0xb4,0xdd,0x4,0xb0,0xa7,0x44,0xd2,0x23,0x31,0x29,0x61,0xe4,0xac,0xb8,0xf1,0x2e,0xb4,0x31,0x44,0x75,0xa0,0xc0,0x7f,0x35,0x6a,0xfb,0x11,0xba,0x6f,0x5f,0x25,0x70,0x37,0xbf,0xd3,0xb6,0x3a,0xaf,0x96,0x5c,0x62,0x16,0xb0,0x82,0xb3,0x9e,0x13,0xfc,0xd2,0xb7,0x59,0x1e,0x98,0x2f,0xe,0x51,0x47,0x14,0xd3,0xba,0x21,0xce,0xfd,0xa0,0xad,0x68,0x5b,0xa8,0x2e,0x8a,0x47,0x46,0xd7,0x30,0x28,0xea,0x25,0x4f,0x75,0x73,0xab};

    void method encode(const char *data, int available, int offset, std::vector<char> &result)
    {
#ifdef DEBUG
//#define DEBUG_NOENCODE
#endif
        result.resize(available);
        for (int i = 0 ; i < available ; ++i) {
#ifdef DEBUG_NOENCODE
            result[i] = data[i];
#else
            result[i] = ~((data[i] ^ kKey[(offset + i) % sizeof(kKey)]) - 76);
#endif
        }
    }

    void method decode(char *data, int available, int offset)
    {
        for (int i = 0 ; i < available ; ++i) {
            data[i] = (~data[i] + 76) ^ kKey[(offset + i) % sizeof(kKey)];
        }
    }
}
