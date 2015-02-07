//
//  OfflineStorage.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 29/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

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
#include "PredicatePlaylist.h"
#include "StoredSongArray.h"
#include "OfflinePlaylist.h"
#include "Encoder.h"
#include "IPreferences.h"

namespace Gear
{
    class GooglePlaySession;
    
#define method OfflineStorage::
    
    using std::string;
    using std::ofstream;
    using std::ios;
    using THREAD_NS::lock_guard;
    using THREAD_NS::mutex;
    
    const std::string OfflineStorage::SourceSessionKey = "sourceService";


    OfflineStorage & method instance()
    {
        static OfflineStorage inst;
        return inst;
    }
    
    static string dataPath()
    {
        auto str = IApp::instance()->preferences().stringForKey("OfflineDownloadLocation");
        if (str.empty()) {
            return IApp::instance()->dataPath();
        }
        return str;
    }

    static string prefix()
    {
        return dataPath() + "/offline-";
    }
    
    static string playlistPrefix()
    {
        return dataPath() + "/offlineplaylist-";
    }
    
    static bool hasEnding (std::string const &fullString, std::string const &ending)
    {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }
    
    method OfflineStorage()
    {
        // this is wrong, it would be make the first song not to play offline...
        //_executor.addTask([this]{
        
        auto filenames = IApp::instance()->fileManager()->listFiles(prefix());
        sort(filenames.begin(), filenames.end());
        vector<std::pair<string,bool>> identifiers;
        identifiers.reserve(filenames.size()/2);

        for (const string &iStr : filenames) {
            if (hasEnding(iStr, ".meta")) {
                continue;
            }
            bool encoded = hasEnding(iStr, ".dat");
            std::string str = iStr;
            if (encoded) {
                str.erase(str.length() - std::string(".dat").length());
            }

            string meta = str + ".meta";
            auto it = lower_bound(filenames.begin(), filenames.end(), meta);
            if (it == filenames.end() || *it != meta) {
                // delete file
                IApp::instance()->fileManager()->deleteFile(prefix() + str);
                continue;
            }
            identifiers.push_back(std::make_pair(str, encoded));
        }
        
        lock_guard<mutex> l(_itemsMutex);
        
        for (auto &nameAndEncoded : identifiers) {
            _items.insert(make_pair(nameAndEncoded.first, std::make_pair(1.0f,nameAndEncoded.second)));
        }
    }
    
    template<class T> T min(T a, T b)
    {
        return a < b ? a : b;
    }
    
    void method update(const string &identifier, float value)
    {
        bool needUpdate;
        {
            lock_guard<mutex> l(_itemsMutex);
            int count = _items.size();
            if (value < 0.0f) {
                auto it =_items.find(identifier);
                if (it != _items.end()) {
                    _items.erase(it);
                }
            } else {
                _items[identifier] = std::make_pair(value, true);
            }
            needUpdate = _items.size() != count;
            
            {
                auto it = _itemStates.find(identifier);
                if (it != _itemStates.end()) {
                    auto s = it->second.lock();
                    if (s) {
                        s->setOffline(value >= 0.0f, value);
                    } else {
                        _itemStates.erase(it);
                    }
                }
            }
        }
        
        if (needUpdate) {
            _updatedEvent.signal();
        }
    }
    
    Base::EventConnector method updatedEvent()
    {
        return _updatedEvent.connector();
    }
    
    static std::string getFilename(const string &identifier, bool encoded)
    {
        return prefix() + identifier + (encoded ? ".dat" : "");
    }
    
    bool method doStoreMeta(const string &identifier, const string &metaString)
    {
        // encoded: false, we need the root here
        string filename = getFilename(identifier, false);
        {
            string metaFilename = filename + ".meta";
            ofstream f(metaFilename.c_str(), ios::out | ios::trunc);
            if (!f.is_open()) {
                //std::cout << "total not open\n";
                return false;
            }
            
            f << metaString;
            f.close();
            
            if (!f.good()) {
                return false;
            }
        }
        return true;
    }

    bool method doStore(const string &identifier, const shared_ptr<IPlaybackData> &data)
    {
        //Recd::Encoder::generateBinaryKeys(2048, 69);
        string filename = getFilename(identifier, true);
        
        // prevent creating file if there is a problem receiving data
        data->waitSync(0);
        if (data->failed()) {
            //std::cout << "total failed\n";
            return false;
        }
        
        {
            ofstream f(filename.c_str(), ios::out | ios::trunc | ios::binary);
            if (!f.is_open()) {
                //std::cout << "total not open\n";
                return false;
            }

            // find out size
            auto length = data->totalLength();
            if (length == 0) {
                return false;
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
                        auto size = min(available, pos-prevPos);

                        encode(ptr, size, prevPos, encoded);
                        f.write(encoded.data(), encoded.size());
                        prevPos += size;
                    });
                    
                    //std::cout << "downloading... " << prevPos << "/" << length << "  real position: " << (int)f.tellp() << std::endl;
                }
                
                update(identifier, ((float)pos)/length);
            };
            
            for (long long i = 1 ; i <= 99 ; i++) {
                pos = i * length /100;
                //std::cout << "downloading " << i << "% bytes: " << pos << std::endl;
                
                data->waitSync(pos);
                if (data->failed()) {
                    return false;
                }
                write();
            }
            //data->waitUntilFinished(length-1);
            pos = length;
            write();

            f.close();
            if (!f.good()) {
                return false;
            }
        }
        return true;
    }

    void method storePlaylist(const std::string &name, const std::vector<string> &songIds)
    {
        {
            string filename = playlistPrefix() + UrlEncode::encode(name);
            ofstream f(filename.c_str(), ios::out | ios::trunc);
            if (!f.is_open()) {
                return;
            }
            f << name << std::endl;
            for (auto &songId : songIds) {
                f << songId << std::endl;
            }
        }
        _updatedEvent.signal();
    }
    
    static vector<string> songIdsForPlaylistId(const string &playlistId, string &name)
    {
        auto path = playlistPrefix() + playlistId;
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
    
    static std::vector<std::string> playlistIds()
    {
        auto filenames = IApp::instance()->fileManager()->listFiles(playlistPrefix());
        return filenames;
    }
    
    void method removePlaylist(const std::string &name)
    {
        // remove songs, but only those which are not present in other playlists!
        auto playlistId = UrlEncode::encode(name);
        auto toDelete = songIdsForPlaylistId(playlistId);
        std::sort(toDelete.begin(), toDelete.end());
        
        for (auto &otherPlaylist : playlistIds()) {
            if (playlistId == otherPlaylist) {
                continue;
            }
            const auto otherSongIds = songIdsForPlaylistId(otherPlaylist);
            for (auto &songId : otherSongIds) {
                auto it = std::lower_bound(toDelete.begin(), toDelete.end(), songId);
                if (it != toDelete.end() && songId == *it) {
                    toDelete.erase(it);
                }
            }
        }
        
        for (auto &songId : toDelete) {
            remove(songId);
        }
        
        IApp::instance()->fileManager()->deleteFile(playlistPrefix() + UrlEncode::encode(name));
        _updatedEvent.signal();
    }
    
    void method store(const string &identifier, const shared_ptr<ISong> &song)
    {
        float ratio = 0;
        bool av = available(identifier, ratio);
        if (av && ratio >= 1.0f) {
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
        {
            auto sourceSession = song->session()->sessionIdentifier();
            values[SourceSessionKey] = Json::Value(sourceSession);
        }
        
        Json::FastWriter writer;
        auto metaString = writer.write(values);
        if (!doStoreMeta(identifier,metaString)) {
            remove(identifier);
            return;
        }
        
        update(identifier, 0.0f);
        
        //std::cout << "store: " << identifier << std::endl;
        
        _executor.addTask([=]{
            
            //std::cout << "dostore: " << identifier << std::endl;
            
            auto data = song->playbackDataSync();
            if (!data) {
                // couldn't fetch data due to error
                remove(identifier);
                return;
            }
            
            if (!doStore(identifier, data)) {
                remove(identifier);
            } else {
                // refresh already existing playlist if necessary
                _updatedEvent.signal();
            }
        });
    }
    
    void method remove(const string &identifier)
    {
        update(identifier, -1);
        
        IApp::instance()->fileManager()->deleteFile(prefix() + identifier);
        IApp::instance()->fileManager()->deleteFile(prefix() + identifier + ".meta");
        
        _updatedEvent.signal();
    }
    
    bool method available(const std::string &identifier) const
    {
        float ratio = 0.0f;
        bool ret = available(identifier, ratio);
        return ret && (ratio > 1.0f-0.00001f);
    }
    
    bool method available(const std::string &identifier, float &ratio) const
    {
        lock_guard<mutex> l(_itemsMutex);
        return availableUnsafe(identifier, ratio);
    }
    
    bool method availableUnsafe(const std::string &identifier, float &ratio) const
    {
        // unsafe because it needs a mutex lock!
        auto it = _items.find(identifier);
        if (it == _items.end()) {
            return false;
        } else {
            ratio = it->second.first;
            return true;
        }
    }
    
    static SongEntry songEntryForIdentifier(const shared_ptr<ISession> &session, const string &identifier)
    {
        #pragma message("TODO: some rework needed on the offline mode")
        return SongEntry();
        /*
        shared_ptr<PlayableSong> song = shared_ptr<PlayableSong>(new PlayableSong(identifier, session));
        std::ifstream f((prefix() + identifier + ".meta").c_str(), ios::in);
        std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();
        if (!f.good()) {
            return SongEntry();
        }
        
        Json::Value json;
        Json::Reader reader;
        if (!reader.parse(str, json) || !json.isObject()) {
            return SongEntry();
        }
        
        for (auto &key : json.getMemberNames()) {
            auto value = json[key].asString();
            song->setStringForKey(key, value);
        }
        return SongEntry(song, identifier);*/
    }
    
    std::vector<shared_ptr<IPlaylist>> method playlists(const shared_ptr<ISession> session)
    {
        auto filenames = playlistIds();
        auto oldPlaylists = _playlists;
        _playlists.clear();
        for (auto &ident : filenames) {
            
            std::string name;
            auto songIds = songIdsForPlaylistId(ident, name);
            if (name.empty() || songIds.empty()) {
                continue;
            }
            
            shared_ptr<OfflinePlaylist> playlist;
            for (auto &p : oldPlaylists) {
                if (p->playlistId() == ident) {
                    playlist = p;
                    break;
                }
            }
            if (!playlist) {
                playlist = OfflinePlaylist::create(ident, name, session);
                //playlist->setRemovable(false);
                playlist->setEditable(false);
                playlist->setSaveForOfflinePossible(false);
            }
            _playlists.push_back(playlist);
            
            vector<SongEntry> entries;
            for (auto &songId : songIds) {
                auto entry = songEntryForIdentifier(session,songId);
                if (entry && available(songId)) {
                    entries.push_back(entry);
                }
            }
            auto array = playlist->storedSongArray();
            #pragma message("TODO: offline playlists display...")
            //array->setAllSongs(entries, false, true);
        }
        vector<shared_ptr<IPlaylist>> ret;
        transform(_playlists.begin(), _playlists.end(), back_inserter(ret), [](const shared_ptr<OfflinePlaylist> &p){
            return p;
        });
        return ret;
    }
    
    shared_ptr<IPlaybackData> method fetch(const std::string &identifier, const IPlaybackData::Format format) const
    {
        bool encoded;
        {
            lock_guard<mutex> l(_itemsMutex);
            auto it = _items.find(identifier);
            if (it == _items.end()) {
                return nullptr;
            }
            encoded = it->second.second;
        }
        return shared_ptr<IPlaybackData>(new FilePlaybackData(getFilename(identifier,encoded), format, encoded));
    }
    
    shared_ptr<OfflineState> method state(const std::string &identifier)
    {
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
        
        float ratio = 0.0f;
        bool offline = availableUnsafe(identifier, ratio);
        shared_ptr<OfflineState> ret(new OfflineState());
        ret->setOffline(offline, ratio);
        auto p = make_pair(identifier, (weak_ptr<OfflineState>)ret);
        //_itemStates.insert(p);
        //_itemStates[identifier] = ret;
        return ret;
    }
                                                   
    std::vector<SongEntry> method allEntries(const shared_ptr<ISession> &session) const
    {
        vector<string> identifiers;
        {
            lock_guard<mutex> l(_itemsMutex);
            for (auto &p : _items) {
                //if (p.second >= 1.0f) {
                identifiers.push_back(p.first);
                //}
            }
        }
        
        std::vector<SongEntry> ret;
        ret.reserve(identifiers.size());
        for (auto &identifier : identifiers) {
            
            SongEntry entry = songEntryForIdentifier(session, identifier);
            if (entry) {
                ret.push_back(entry);
            }
        }
        
        return ret;
    }

    static unsigned char kKey[256] = {0x82,0x94,0x72,0x83,0xd7,0x2a,0x31,0x7a,0x95,0x6a,0x5b,0xdc,0xc5,0x43,0x16,0xd8,0x26,0x6a,0xcb,0x4a,0x18,0x8d,0x31,0x59,0xd5,0xdf,0xc4,0x52,0x42,0x5e,0x70,0xd0,0x7,0xf9,0x51,0x9d,0xac,0x58,0xbd,0xc4,0x52,0x91,0xff,0x70,0xe0,0xf9,0x2,0x85,0xf9,0x41,0xc7,0xcb,0xb0,0xeb,0x77,0x12,0xa,0x5d,0x4,0x82,0x61,0x92,0xb6,0x27,0xe,0x96,0xab,0xc0,0x3a,0x9f,0xd0,0xee,0x6a,0x5b,0xdb,0xb8,0xfe,0xf4,0xe9,0xdc,0x28,0x3c,0x25,0x67,0x27,0x48,0x81,0x2f,0xde,0xf9,0x2,0xbc,0x64,0x35,0x6a,0x98,0x2b,0xee,0xd7,0xf3,0x8b,0x41,0x7b,0x6f,0x3,0xdd,0x86,0x98,0x46,0x60,0xb4,0x7e,0x4d,0x75,0x54,0xc6,0x7a,0xde,0xd8,0xa2,0x51,0xe7,0x99,0x51,0xbc,0x3b,0xf3,0xfe,0x11,0xf1,0xd5,0x31,0x15,0x4e,0x32,0xd,0x93,0xd2,0xd2,0x59,0x8c,0x53,0xae,0xb8,0x56,0x73,0x67,0x50,0x32,0x5b,0x3f,0x4e,0x6b,0xae,0x36,0xb6,0xf8,0x6e,0xe,0xd3,0x28,0x21,0x93,0x45,0x17,0x88,0xe8,0x41,0xdb,0xf1,0x3,0xc5,0x37,0x13,0xc6,0xb4,0xdd,0x4,0xb0,0xa7,0x44,0xd2,0x23,0x31,0x29,0x61,0xe4,0xac,0xb8,0xf1,0x2e,0xb4,0x31,0x44,0x75,0xa0,0xc0,0x7f,0x35,0x6a,0xfb,0x11,0xba,0x6f,0x5f,0x25,0x70,0x37,0xbf,0xd3,0xb6,0x3a,0xaf,0x96,0x5c,0x62,0x16,0xb0,0x82,0xb3,0x9e,0x13,0xfc,0xd2,0xb7,0x59,0x1e,0x98,0x2f,0xe,0x51,0x47,0x14,0xd3,0xba,0x21,0xce,0xfd,0xa0,0xad,0x68,0x5b,0xa8,0x2e,0x8a,0x47,0x46,0xd7,0x30,0x28,0xea,0x25,0x4f,0x75,0x73,0xab};

    void method encode(const char *data, int available, int offset, std::vector<char> &result)
    {
        result.resize(available);
        for (int i = 0 ; i < available ; ++i) {
            result[i] = ~((data[i] ^ kKey[(offset + i) % sizeof(kKey)]) - 76);
        }
    }

    void method decode(char *data, int available, int offset)
    {
        for (int i = 0 ; i < available ; ++i) {
            data[i] = (~data[i] + 76) ^ kKey[(offset + i) % sizeof(kKey)];
        }
    }
}
