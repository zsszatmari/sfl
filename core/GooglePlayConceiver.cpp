//
//  GooglePlayConceiver.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#include "GooglePlayConceiver.h"
#include "GooglePlaySession.h"
#include "Artist.h"
#include "Album.h"
#include "GooglePlayRadio.h"
#include "ModifiablePlaylist.h"

namespace Gear
{
    using std::map;

#define method GooglePlayConceiver::
    
    method GooglePlayConceiver(const Json::Value &value, const weak_ptr<GooglePlaySession> &session, int hint) :
            IConceiver(session),
            _json(value),
            _hint(static_cast<Hint>(hint)),
            _session(session),
            _starRatedPresent(false)
    {
    }
    
    static vector<SongEntry> entriesFromIds(const vector<string> &ids)
    {
        vector<SongEntry> ret;
        ret.reserve(ids.size());
        
        transform(ids.begin(), ids.end(), back_inserter(ret), [](const string &str){
            return SongEntry(GooglePlaySession::sourceIdentifier, "", str, str);
            //return SongEntry(shared_ptr<ISong>(), str);
        });
        return ret;
    }
    
    map<string,int> method keysToIndexesTable()
    {
        // only for the fields we wish to edit (upload changes)
        map<string,int> ret;
        ret.insert(std::make_pair("title", 1));
        ret.insert(std::make_pair("artist", 3));
        ret.insert(std::make_pair("album", 4));
        ret.insert(std::make_pair("albumArtist", 5));
        ret.insert(std::make_pair("track", 14));
        ret.insert(std::make_pair("disc", 16));
        ret.insert(std::make_pair("year", 18));
        // no to be updated this way
        //ret.insert(std::make_pair("playCount", 22));
        //ret.insert(std::make_pair("lastPlayed", 35));
        ret.insert(std::make_pair("genre", 11));
        ret.insert(std::make_pair("rating", 23));
        
        return ret;
    }
    
    const vector<SongEntry> method songs(const string &playlistId) const
    {
        vector<SongEntry> ret;
        
        if (_hint != Hint::loadLibrary) {
            if (_json.size() < 2) {
                return ret;
            }
        }
        
        Json::Value songs;
        switch(_hint) {
            case Hint::loadPlaylist:
            case Hint::search:
            case Hint::fetchRadio: {
                
                auto &root = _json[1];
 
                if (root.size() < 1) {
                    return ret;
                }
                songs = root[0];
            }
                break;
            case Hint::loadLibrary: {
                songs = _json;
                break;
            }
            case Hint::fetchArtist: {
                
                auto &root = _json[1];
                
                if (root.size() < 1) {
                    return ret;
                }
                auto &inner1 = root[0];
                if (inner1.size() < 1) {
                    return ret;
                }
                auto &inner2 = inner1[0];
                if (inner2.size() < 1) {
                    return ret;
                }
                songs = inner2[inner2.size() -1];
                break;
            }
            case Hint::fetchAlbum: {
                
                auto &root = _json[1];
                
                if (root.size() < 1) {
                    return ret;
                }
                auto &inner1 = root[0];
                if (inner1.size() < 1) {
                    return ret;
                }
                auto &inner2 = inner1[0];
                if (inner2.size() < 7) {
                    return ret;
                }
                songs = inner2[6];
                break;
            }
            case Hint::radioListUser: {
                
                auto &root = _json[1];
                
                if (root.size() < 2) {
                    return ret;
                }
                songs = root[1];
                break;
            }
            case Hint::createPlaylist:
            case Hint::addToPlaylist:
                return entriesFromIds(ids());
            case Hint::addToLibrary: {
                
                auto &root = _json[1];
                if (root.size() < 2) {
                    return ret;
                }
                auto &songs = root[1];
                
                for (auto itPair = songs.begin() ; itPair != songs.end() ; ++itPair) {
                	auto pair = *itPair;
                    if (pair.size() < 2) {
                        continue;
                    }
                    auto libraryId = pair[0].asString();
                    auto allAccessId = pair[1].asString();
                    
                    SongEntry entry(GooglePlaySession::sourceIdentifier, playlistId, libraryId, allAccessId);
                    shared_ptr<ISong> song = entry.song();
                    // add library from radio, then thumbs up, then there are duplicates in he auto playlist without this!
                    song->setStringForKey("matchedKey", allAccessId);
                    ret.push_back(entry);
                }
                return ret;
            }
        }
        
        ret.reserve(songs.size());
        for (auto itSongJson = songs.begin() ; itSongJson != songs.end() ; ++itSongJson) {
            Json::Value &songJsonPure = *itSongJson;

            auto songJsonDetermine = [&]() -> Json::Value & {
                static Json::Value empty;
                if (_hint == Hint::radioListUser) {
                    if (songJsonPure.size() < 5) {
                        return empty;
                    }
                    auto &inner = songJsonPure[4];
                    if (inner.size() < 1) {
                        return empty;
                    }
                    return inner[0];
                } else {
                    return songJsonPure;
                }
            };
            auto &songJson = songJsonDetermine();
            
            if (songJson.size() < 34) {
                continue;
            }
            
            // will be called with 'mjck' argument
            const string googleId = songJson[0].asString();
            string uniqueId = googleId;
            
            string playlistEntryId;
            if (_hint == Hint::loadPlaylist && songJson.size() > 43) {
                playlistEntryId = songJson[43u].asString();
            } else {
                playlistEntryId = uniqueId;
            }
            SongEntry entry(GooglePlaySession::sourceIdentifier, playlistId, playlistEntryId, uniqueId);
            auto song = entry.song();
            
            // static causes a crash on exit
            //static map<string,int> keysToIndexes = keysToIndexesTable();
            static map<string,int> *pKeysToIndexes = new map<string,int>(keysToIndexesTable());
            map<string,int> &keysToIndexes = *pKeysToIndexes;

            song->setStringForKey("title", songJson[keysToIndexes["title"]].asString());
            string providedArt = songJson[2].asString();
            //std::cout << "provided art: '" << providedArt << "'\n";
            if (providedArt.empty() || providedArt == "0") {
                // avoid 'http:0'
                song->setStringForKey("albumArtUrl", "");
            } else {
                std::string kHttp("http:");
                std::string kHttps("https:");
                if (providedArt.compare(0, kHttp.length(), kHttp) == 0 || 
                    providedArt.compare(0, kHttps.length(), kHttps) == 0) {
                    song->setStringForKey("albumArtUrl", providedArt);
                } else {
                    song->setStringForKey("albumArtUrl", kHttp + providedArt);
                    
                }                
#ifdef DEBUG
                string debUrl = song->albumArtUrl();
#endif
                
            }
            song->setStringForKey("artist", songJson[keysToIndexes["artist"]].asString());
            song->setStringForKey("album", songJson[keysToIndexes["album"]].asString());
		    // 11 genre?
            auto matchedKey = songJson[28].asString(); // 27 is wrong
            if (matchedKey != "0") {
            	song->setStringForKey("matchedKey", matchedKey);
            }
            song->setStringForKey("albumArtist", songJson[keysToIndexes["albumArtist"]].asString());
            song->setStringForKey("albumId", songJson[32].asString());
            song->setStringForKey("artistId", songJson[33].asString());
            song->setUintForKey("durationMillis", songJson[13].asUInt64());
            song->setUintForKey("track", songJson[keysToIndexes["track"]].asUInt64());
            song->setUintForKey("disc", songJson[keysToIndexes["disc"]].asUInt64());
            song->setUintForKey("year", songJson[keysToIndexes["year"]].asUInt64());
            
            auto playCount = songJson[22].asUInt64();
            song->setUintForKey("playCount", playCount);
            
            // it is not the 35th, it does not change on play!
            if (playCount > 0 && songJson.size() >= 49) {
                song->setUintForKey("lastPlayed",songJson[48].asUInt64());
            }
            song->setUintForKey("creationDate",songJson[24].asUInt64());
            
            auto genre = songJson[keysToIndexes["genre"]].asString();
            //std::cout << "got genre: " << genre << std::endl;
            if (genre == "0") {
                genre = "";
            }
            song->setStringForKey("genre", genre);
            
            int64_t rating = songJson[keysToIndexes["rating"]].asInt64();
            if (rating >= 0 && rating <= 5) { // not -1
                if (rating > 1 && rating < 5) {
                    _starRatedPresent = true;
                }
                song->setUintForKey("rating", rating);
            }
            
            entry.song()->save();
            ret.push_back(entry);
        }
        return ret;
    }

    bool method starRatedPresent() const
    {
        return _starRatedPresent;
    }
    
    const vector<shared_ptr<IArtist>> method artists() const
    {
        vector<shared_ptr<IArtist>> ret;
        if (_json.size() < 2) {
            return ret;
        }
        auto root = _json[1];
        if (root.size() < 3) {
            return ret;
        }
        auto artists = root[2];
        
        ret.reserve(artists.size());
        for (auto itArtistJson = artists.begin() ; itArtistJson != artists.end() ; ++itArtistJson) {
        	auto artistJson = *itArtistJson;

        	if (artistJson.size() < 2) {
                continue;
            }
            
            string uniqueId = artistJson[1].asString();
            
            shared_ptr<Artist> artist(new Artist(uniqueId));
            ret.push_back(artist);
        }
        return ret;
    }
    
    const vector<shared_ptr<IAlbum>> method albums() const
    {
        vector<shared_ptr<IAlbum>> ret;
        
        if (_json.size() < 2) {
            return ret;
        }
        auto &root = _json[1];
        
        vector<Json::Value> albumJsons;
        auto addTo = [&albumJsons](const Json::Value &add){
        	for (auto itAlbumJson = add.begin() ; itAlbumJson != add.end() ; ++itAlbumJson) {
                albumJsons.push_back(*itAlbumJson);
            }
        };
        
        switch(_hint) {
            case Hint::search:
            case Hint::fetchAlbum: {
                if (root.size() < 3) {
                    return ret;
                }
                addTo(root[1]);
                if (root.size() >= 6) {
                    albumJsons.push_back(root[5]);
                }
                break;
            }
            case Hint::fetchArtist: {
                if (root.size() < 1) {
                    return ret;
                }
                auto &inner1 = root[0];
                if (inner1.size() < 1) {
                    return ret;
                }
                auto &inner2 = inner1[0];
                if (inner2.size() < 5) {
                    return ret;
                }
                
                addTo(inner2[4]);
                break;
            }
            case Hint::radioListUser:
            case Hint::fetchRadio:
            case Hint::createPlaylist:
            case Hint::addToPlaylist:
            case Hint::addToLibrary:
            case Hint::loadLibrary:
            case Hint::loadPlaylist:
                // nada
                break;
        }
        
        
        ret.reserve(albumJsons.size());
        for (auto itAlbumJson = albumJsons.begin() ; itAlbumJson != albumJsons.end() ; ++itAlbumJson) {
        	auto albumJson = *itAlbumJson;

        	if (albumJson.size() < 8) {
                continue;
            }
            
            string uniqueId = albumJson[7].asString();
            
            shared_ptr<Album> artist(new Album(uniqueId));
            ret.push_back(artist);
        }
        return ret;
    }
    
    static const vector<shared_ptr<IPlaylist>> playlistsFrom(const Json::Value &playlists, const shared_ptr<GooglePlaySession> &session)
    {
        #pragma message("TODO: we have to filter library-based radios based on library availability (what does this mean?)")
        auto songArray = session->librarySongArray();
        /*ValidPtr<const vector<SongEntry>> songs;
        if (songArray) {
            songs = songArray->rawSongs();
        }*/
        vector<string> libraryIds;
        /*libraryIds.reserve(songs->size());
        transform(songs->begin(), songs->end(), back_inserter(libraryIds), [](const SongEntry &entry){
            return entry.entryId();
        });*/
        sort(libraryIds.begin(), libraryIds.end());
        
        vector<shared_ptr<IPlaylist>> ret;
        ret.reserve(playlists.size());

        for (auto itPlaylist = playlists.begin() ; itPlaylist != playlists.end() ; ++itPlaylist) {
        	auto playlist = *itPlaylist;
            
            // invalid:
            
            /*["1ca86dfe-fb04-3a0a-9924-21dd037145ee","Some Things You Can't Return","",[]
             ,[]
             ,1371830137782000,[0,"4791ca77-c1a1-3de2-9623-4ebb17ce2514"]
            */
            if (playlist.size() < 7) {
                continue;
            }
            
            auto source = playlist[6];
            if (source.size() < 2) {
                continue;
            }
            if (source[0].asInt() == 0) {
                auto songId = source[1].asString();
                
                auto bound = lower_bound(libraryIds.begin(), libraryIds.end(), songId);
                if ((bound == libraryIds.end() || *bound != songId) && songArray) {
                    continue;
                }
            }
            
            shared_ptr<GooglePlayRadio> radio =  GooglePlayRadio::create(playlist[1].asString(), playlist[0].asString(), session);
            ret.push_back(radio);
        }
        return ret;
    }
    
    const vector<shared_ptr<IPlaylist>> method playlists() const
    {
        vector<shared_ptr<IPlaylist>> ret;
        
        if (_json.size() < 2) {
            return ret;
        }
        auto &root = _json[1];
        
        if (root.size() < 2) {
            return ret;
        }
        
        auto session = _session.lock();
        if (!session) {
            return ret;
        }
        
        if (_hint == Hint::radioListUser) {
            
            return playlistsFrom(root[0], session);

        } else if (_hint == Hint::fetchRadio) {
            
            auto &str = root[1];
            
            shared_ptr<GooglePlayRadio> radio = GooglePlayRadio::create("", str.asString(), session);
            ret.push_back(radio);
        } else if (_hint == Hint::createPlaylist) {
            
            auto &str = root[0];
            const string playlistId = str.asString();

            auto playlist = ModifiablePlaylist::create(playlistId,"", session);
            ret.push_back(playlist);
        }
        
        return ret;
    }
    
    const vector<string> method ids() const
    {
        vector<string> ret;
        

        if (_hint == Hint::createPlaylist) {
            
            if (_json.size() < 2) {
                return ret;
            }
            auto &root = _json[1];
            
            if (root.size() < 3) {
                return ret;
            }
            
            auto &jsonArray = root[2];
            for (auto itJson = jsonArray.begin() ; itJson != jsonArray.end() ; ++itJson) {
            	auto jsonElement = *itJson;
                if (jsonElement.size() < 3) {
                    continue;
                }
                
                ret.push_back(jsonElement[2].asString());
            }
        } else if (_hint == Hint::addToPlaylist) {
            auto &jsonArray = _json["songIds"];
            
            for (auto itJson = jsonArray.begin() ; itJson != jsonArray.end() ; ++itJson) {
                auto jsonElement = *itJson;
                auto &idElement = jsonElement["playlistEntryId"];
                const auto str = idElement.asString();
                if (!str.empty()) {
                    ret.push_back(str);
                }
            }
        }
        return ret;
    }
}
