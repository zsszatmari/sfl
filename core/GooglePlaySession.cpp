//
//  GooglePlaySession.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#include <algorithm>
#include <iostream>
#include "GooglePlaySession.h"
#include "GooglePlayConceiver.h"
#include "json.h"
#include "IAlbum.h"
#include "MainExecutor.h"
#include "BaseUtility.h"
#include "Color.h"
#include "MusicSortCompare.h"
#include "CoreDebug.h"
#include "Tracking.h"
#include "sfl/Prelude.h"
#include "sfl/Vector.h"
#include "GooglePlaySessionImpl.h"

namespace Gear
{
    using std::function;
    using std::map;

    const std::string GooglePlaySession::sourceIdentifier = "gplay";

    
#define method GooglePlaySession::
    
    using std::cerr;
    using namespace Base;
    using namespace Gui;
    using namespace sfl;
    
    static const vector<PlaylistCategory> emptyPlaylistsByCategories()
    {
        PlaylistCategory radio = PlaylistCategory(u("Radio"), kRadioTag);
        PlaylistCategory all = PlaylistCategory(u(kAllCategory), kAllTag, true);
        all.setSingularPlaylist(false);
        
        
        //PlaylistCategory artists = PlaylistCategory(u(kArtistsCategory), kArtistsTag);
        PlaylistCategory playlists = PlaylistCategory(u(kPlaylistsCategory), kPlaylistsTag);
        PlaylistCategory autoPlaylists = PlaylistCategory(u(kAutoPlaylistsCategory), kAutoPlaylistsTag);
        PlaylistCategory pa[] = {/*freesearch, */all, /*artists,*/ playlists, radio, autoPlaylists};
        
        return init<vector<PlaylistCategory>>(pa);
    }
    
#ifdef DEBUG
    static int count = 0;
#endif

    DbSession::Behaviour method behaviour(GooglePlaySession *nself)
    {
        DbSession::Behaviour behaviour;
        behaviour.sourceIdentifier = GooglePlaySession::sourceIdentifier;
        behaviour.name = "Google Play";
        behaviour.connect = [=](const string &user, const string &pass, const function<void(bool)> &callback){
            auto self = nself->shared_from_this();
            // connectSync blocks, which is a nasty thing to do
            self->connect(user,pass,callback);
        };
        behaviour.fetchLibrary = [=](const function<bool(const string &,const vector<Json::Value> &,bool)> &callback){
            #pragma message("TODO: delete legacy db")
            #pragma message("TODO: shared playlist do not work")
            #pragma message("TODO: fetch ephemeral thumbs up list")

            auto self = nself->shared_from_this();

            self->radioList([self,callback](const vector<Json::Value> &radios){
                if (!callback("Playlist",radios,false)) {
                    return;
                }
                self->fetchLibrary([callback](const vector<Json::Value> &songs)->bool{
#ifdef DEBUG
                    std::cout << "fetched " << songs.size() << " google library songs\n";
#endif
                    return callback("Song",songs,false);
                },[self,callback]{
                    self->fetchPlaylists(callback, [callback]{
                        callback("Song",vector<Json::Value>(),true);
                        callback("Playlist",vector<Json::Value>(),true);
                    });
                });
            });
        };
        behaviour.presence = [nself]{
            Json::Value ret;
            ret["all"] = "My Library";
            ret["free"] = "All Access";
            ret["playlist"] = "Playlists";
            ret["radio"] = nself->allAccessCapable() ? "Radio" : "Instant Mix";
            return std::move(ret);
        };
        behaviour.icon = "favicon-gplay";
        behaviour.playback = [=](const string &uniqueId, const function<void(const shared_ptr<IPlaybackData> &)> &result){
            auto self = nself->shared_from_this();
            
            result(self->playbackDataSync(uniqueId));
        };
        behaviour.fetchSearch = [nself](const string &filter,const Json::Value &context, const DbSession::Behaviour::FetchSearchCallback &callback){
            if (nself->allAccessCapable()) {
                auto self = nself->shared_from_this();
                self->search(filter,context,callback);
            }
        };
        behaviour.fetchDynamic = [nself](const string &playlistId, const function<void(const vector<Json::Value> &)> &callback){
            auto self = nself->shared_from_this();
            nself->fetchRadioFeed(playlistId, [playlistId,callback,self](const shared_ptr<Json::Value> &json){
                if (json) {
#ifdef DEBUG
                    //Json::FastWriter writer;
                    //std::cout << "got radio: " << writer.write(*json) << std::endl;
#endif
                    auto conceiver = self->_conceiverFactory->conceiver(*json, self);
                    callback(conceiver->songs(playlistId));
                }
            });
        };
        behaviour.removePlaylist = [nself](const string &playlistId, const std::map<string,string> &traits){
            if (traits.find("dynamic") == traits.end()) {
                nself->removePlaylist(playlistId);
            } else {
                nself->removeRadio(playlistId);
            }
        };
        return behaviour;
    }

    shared_ptr<GooglePlaySessionImpl> method shared_from_this()
    {
        return MEMORY_NS::static_pointer_cast<GooglePlaySessionImpl>(DbSession::shared_from_this());
    }

    method GooglePlaySession(const shared_ptr<const IConceiverFactory<GooglePlaySession>> &conceiverFactory, const shared_ptr<IApp> &app) :
        DbSession(app, behaviour(this)),
        _conceiverFactory(conceiverFactory)
    {
#ifdef DEBUG
        ++count;
#endif
        _categories = emptyPlaylistsByCategories();
    }
    
    method ~GooglePlaySession()
    {
#if DEBUG
        --count;
        //cout << "destructing session.." << endl;
#endif
    }
    
   #pragma message("TODO: ephemeral thumbs up songs, see GooglePlayThumbsPlaylist")
    
    bool method allAccessCapable() const
    {
        return _allAccessCapable;
    }
    
    void method setAllAccessCapable(bool allAccessCapable)
    {
        _allAccessCapable = allAccessCapable;
    }

    static vector<string> jsonToVector(const Json::Value &json)
    {
        vector<string> ret;
        if (!json.isArray()) {
            return ret;
        }

        for (int i = 0 ; i < json.size() ; ++i) {
            ret.push_back(json[i].asString());
        }
        return ret;
    }

    static Json::Value vectorToJson(const vector<string> &v)
    {
        Json::Value ret(Json::arrayValue);
        for (auto &item : v) {
            ret.append(item);
        }
        return ret;
    }

    void method search(const string &filter, const Json::Value &context, const DbSession::Behaviour::FetchSearchCallback &callback)
    {
        if (context.isNull()) {
            return;
        }
        bool gotFirst = context["user"]["gotFirst"].asBool();
        auto self = shared_from_this();

        if (!gotFirst) {

            searchAllAccess(filter,[filter,context,callback,self,this](const shared_ptr<Json::Value> &json) {
                if (!json) {
                    return;
                }
                auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this());
                auto ret = conceiver->songs("temp:free");
                
                auto artistList = conceiver->artists();
                auto albumList = conceiver->albums();

                Json::Value artists(Json::arrayValue);
                for (auto &artist : artistList) {
                    artists.append(artist->uniqueId());
                }
                Json::Value albums(Json::arrayValue);
                for (auto &album : albumList) {
                    albums.append(album->uniqueId());
                }
                
                artistsToFetch = deque<shared_ptr<IArtist>>(artistList.begin(), artistList.end());
                albumsToFetch = deque<shared_ptr<IAlbum>>(albumList.begin(), albumList.end());

                auto newContext = context;
                newContext["user"]["gotFirst"] = true;
                newContext["user"]["artistsToFetch"] = artists;
                newContext["user"]["albumsToFetch"] = albums;

                search(filter,callback(newContext,ret),callback);
            });
            
        } else {

            auto artistsToFetch = jsonToVector(context["user"]["artistsToFetch"]);
            auto albumsToFetch = jsonToVector(context["user"]["albumsToFetch"]);
                        
            if (!artistsToFetch.empty()) {
                auto artist = head(artistsToFetch);
                artistsToFetch = tail(artistsToFetch);
                
                fetchArtist(artist,[artistsToFetch,albumsToFetch,context,filter,self,this,callback](const shared_ptr<Json::Value> &json) mutable {
                    auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this(),static_cast<int>(GooglePlayConceiver::Hint::fetchArtist));
                    auto ret = conceiver->songs("temp:free");
                    vector<string> additionalAlbums = sfl::map([](const shared_ptr<IAlbum> &album){return album->uniqueId();},conceiver->albums());

                    // avoid duplicates
                    auto eraseIt = remove_if(additionalAlbums.begin(), additionalAlbums.end(), [&](const string &item){
                        for (auto it = albumsToFetch.begin() ; it != albumsToFetch.end() ; ++it) {
                            auto &rhs = *it;

                            if (rhs == item) {
                                return true;
                            }
                        }
                        return false;
                    });
                    additionalAlbums.erase(eraseIt, additionalAlbums.end());
                    copy(additionalAlbums.begin(), additionalAlbums.end(), back_inserter(albumsToFetch));
                    
                    auto newContext = context;
                    newContext["user"]["artistsToFetch"] = vectorToJson(artistsToFetch);
                    newContext["user"]["albumsToFetch"] = vectorToJson(albumsToFetch);
                    
                    search(filter,callback(newContext,ret),callback);
                });
                
            } else if (!albumsToFetch.empty()) {
                auto album = head(albumsToFetch);
                albumsToFetch = tail(albumsToFetch);
                
                fetchAlbum(album,[context,filter,self,this,callback,albumsToFetch](const shared_ptr<Json::Value> &json){
                    auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this(), static_cast<int>(GooglePlayConceiver::Hint::fetchAlbum));
                    auto ret = conceiver->songs("temp:free");
                    
                    auto newContext = context;
                    newContext["user"]["albumsToFetch"] = vectorToJson(albumsToFetch);
                    
                    search(filter,callback(newContext,ret),callback);
                });
                
            } else {
                // finished
            }
        }
    }
    
    /*
    const vector<Json::Value> method radioSongsSync(const string &radioId)
    {
        auto json = fetchRadioFeed(radioId);
        auto pThis = shared_from_this();
        
        auto conceiver = _conceiverFactory->conceiver(*json, pThis);
        return conceiver->songs("temp:" + radioId);
    }*/
    
    /*
    const vector<Json::Value> method sharedSongsSync(const string &playlistId)
    {
        auto json = fetchSharedSongs(playlistId);
        auto pThis = shared_from_this();
        auto conceiver = _conceiverFactory->conceiver(*json, pThis);
        return conceiver->songs(playlistId);
    }*/
    
    /*
    const vector<Json::Value> method ratedSongsSync()
    {
        auto json = fetchRatedSongs();
        auto pThis = shared_from_this();
        auto conceiver = _conceiverFactory->conceiver(*json, pThis);
        // not temporary! otherwise we couldn't merge
        return conceiver->songs("ephemthumbsup");   
    }*/
                            
    bool method isAllAccessId(const string &str)
    {
        return str.find("-") == string::npos;
    }
    
    void method addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
    {
#pragma message("TODO: adding all access songs to library or playlist should work")
#if 0
        vector<string> songIds;
        for (auto it = songs.begin() ; it != songs.end() ; ++it) {
        	auto &song = *it;

        	auto unique = song->uniqueId();

            if (isAllAccessId(unique)) {
                auto directId = unique;
                songIds.push_back(directId);
            }
        }
        
        if (!songIds.empty()) {
            
            auto pThis = shared_from_this();
            // let user add to library while during initial fetch...

            pThis->addAllAccessIdsToLibrary(songIds,[songs,pThis](const shared_ptr<Json::Value> &json){
                auto conceiver = pThis->_conceiverFactory->conceiver(*json, pThis, static_cast<int>(GooglePlayConceiver::Hint::addToLibrary));
                auto gotSongs = conceiver->songs("all");
                
                vector<SongEntry> newSongs;
                
                for (auto itEntry = gotSongs.begin() ; itEntry != gotSongs.end() ; ++itEntry) {
                	auto &gotEntry = *itEntry;

                	auto allAccessId = gotEntry["id"].asString();
                    auto libraryId = gotEntry["entryId"].asString();
                    
                    shared_ptr<ISong> foundOriginalSong;

                    for (auto itOriginal = songs.begin() ; itOriginal != songs.end() ; ++itOriginal) {
                    	auto &originalSong = *itOriginal;

    					if (originalSong->uniqueId() == allAccessId) {
                            foundOriginalSong = originalSong;
                            break;
                        }
                    }
                    if (foundOriginalSong) {
                    	if (pThis->_allAccessCapable) {
                    		libraryId = allAccessId;
                    	}
                        SongEntry newEntry(foundOriginalSong, libraryId);
    					if (!pThis->_libraryPlaylist->songArray()->contains(newEntry)) {
    						newSongs.push_back(newEntry);
    					}
                    }
                }
            });
            // pThis->_libraryPlaylist->addSongsLocally(newSongs);
        }
#endif
    }
    
    ValidPtr<const vector<PlaylistCategory>> method categories() const
    {
        return _categories;
    }
    
    void method dispose()
    {
    }
    
    void method radioList(const function<void(const vector<Json::Value> &)> &results)
    {
        auto factory = _conceiverFactory;
        auto self = shared_from_this();
        loadRadio([results,factory,self](const shared_ptr<Json::Value> &json){
            auto conceiver = factory->conceiver(*json, self, static_cast<int>(GooglePlayConceiver::Hint::radioListUser));
            
            auto ret = conceiver->playlists();

#pragma message("TODO: I'm feeling lucky radio")
            Json::Value feelinglucky;
            feelinglucky["playlistId"] = "lucky";
            feelinglucky["name"] = "I'm Feeling Lucky";
            feelinglucky["dynamic"] = true;
            ret.insert(ret.begin(), feelinglucky);

            for (auto &item : ret) {
                item["category"] = "radio";
                item["dynamic"] = true;
            }
            
            results(std::move(ret));
        });
    }
    
    SerialExecutor & method executor()
    {
        return _executor;
    }
    
    void method modifySongs(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts)
    {
#pragma message("TODO: modify ratings, song metadata, and playlist position")
/*
        bool ratingChanged = false;
        int newRating;
        for (auto &changedInt : changedInts) {
            if (changedInt.first == "rating") {
                ratingChanged = true;
                newRating = changedInt.second;
                break;
            }
        }
        if (ratingChanged) {
            _thumbsUpEphemeral->reset();
        }
         
    	// hacky formalism needed by tizen2.2 / gcc ...
        auto pThis = shared_from_this();
        auto copiedIds = ids;
        auto copiedStrings = changedStrings;
        auto copiedInts = changedInts;
        auto l = [pThis, copiedIds,copiedStrings, copiedInts]{
            pThis->modifySongsSync(copiedIds, copiedStrings, copiedInts);
        };
        function<void()> f = l;

        auto &e = executor();
        e.addTask(f);*/
    }
    
    /*
    const shared_ptr<IPlaylist> method createRadioSync(shared_ptr<ISong> &song, const string &fieldHint)
    {
        auto pThis = this;
        string useId;
        int type;
        string radioName;
        if (fieldHint == "artist") {
            useId = song->artistId();
            radioName = song->artist();
            type = 3;
        } else if (fieldHint == "album") {
            useId = song->albumId();
            radioName = song->album();
            type = 2;
        } else {
            useId = song->uniqueId();
            radioName = song->title();
            if (isAllAccessId(useId)) {
                type = 1;
            } else {
                type = 0;
            }
        }
                
        auto json = pThis->createRadio(useId, radioName, type);
        auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this(), static_cast<int>(GooglePlayConceiver::Hint::fetchRadio));
        
        //auto songs = conceiver->songs();
        auto playlists = conceiver->playlists();
        if (playlists.size() > 0) {
            auto newPlaylist = playlists.at(0);
            newPlaylist["name"] = radioName;
            
#pragma message("TODO: create radio")
            //return newPlaylist;
            return shared_ptr<IPlaylist>();
        }
        return shared_ptr<IPlaylist>();
    }*/
    
    static const vector<string> songIdsFromSongs(const vector<shared_ptr<ISong>> &songs)
    {
        vector<string> songIds;
        songIds.reserve(songs.size());
        transform(songs.begin(), songs.end(), back_inserter(songIds), [](const shared_ptr<ISong> &rhs){
            return rhs->uniqueId();
        });
        return songIds;
    }
    
    vector<SongEntry> method mixSongsAndPlaylistIds(const vector<shared_ptr<ISong>> &songs, const vector<Json::Value> entriesForIds)
    {
        if (songs.size() != entriesForIds.size()) {
            return vector<SongEntry>();
        }
        
        vector<SongEntry> newEntries;
        newEntries.reserve(songs.size());
        
        auto itId = entriesForIds.begin();
        transform(songs.begin(), songs.end(), back_inserter(newEntries), [&itId](const shared_ptr<ISong> &song) {
            
            SongEntry entry(song, (*itId)["entryId"].asString());
            ++itId;
            return entry;
        });
        
        return newEntries;
    }
    
    void method addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result)
    {        
        vector<string> songIds = songIdsFromSongs(songs);
        
        auto self = shared_from_this();
        addSongsToPlaylist(playlistId, songIds, [result,self,this,playlistId,songs](const shared_ptr<Json::Value> &json){
            if (!json) {
                result(vector<SongEntry>());
                return;
            }
            auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this(), static_cast<int>(GooglePlayConceiver::Hint::addToPlaylist));
            
            result(mixSongsAndPlaylistIds(songs, conceiver->songs(playlistId)));
        });
    }
    
    void method createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result)
    {        
#pragma message("TODO: create user playlist")
#if 0
        auto pThis = shared_from_this();
        
        
        vector<string> songIds = songIdsFromSongs(songs);
        pThis->createPlaylist(songIds, name, [result,name,pThis,this,songs](const shared_ptr<Json::Value> &json){
            auto conceiver = _conceiverFactory->conceiver(*json, pThis, static_cast<int>(GooglePlayConceiver::Hint::createPlaylist));
            auto playlists = conceiver->playlists();
            if (playlists.size() < 1) {
                result(shared_ptr<IPlaylist>());
                return;
            }
            auto playlistForId = playlists.at(0);
            
            auto playlistId = playlistForId["playlistId"].asString();
            auto playlist = ModifiablePlaylist::create(playlistId, name, pThis);
            auto songsForIds = conceiver->songs(playlistId);
            
            if (songsForIds.size() != songs.size()) {
                result(shared_ptr<IPlaylist>());
                return;
            }
            
            vector<SongEntry> newEntries = mixSongsAndPlaylistIds(songs, songsForIds);
            
            #pragma message("TODO: ensure that local playlist creation works")
            //playlist->addSongsLocally(newEntries);
            
			const pair<string, shared_ptr<IPlaylist>> p = make_pair(playlist->playlistId(), playlist);

            MainExecutor::instance().addTask([pThis, playlist, result,p] () {
                
                pThis->app()->editPlaylistName(playlist);
                pThis->playlistsChanged();
                
                result(playlist);
            });
        });
#endif
    }
    
    void method changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &after, const SongEntry &before)
    {
        vector<string> songIds;
        vector<string> entryIds;
        songIds.reserve(songs.size());
        entryIds.reserve(songs.size());
        for (auto itEntry = songs.begin() ; itEntry != songs.end() ; ++itEntry) {
        	auto &entry = *itEntry;

        	songIds.push_back(entry.song()->uniqueId());
            entryIds.push_back(entry.entryId());
        }
        
        changePlaylistOrderSync(playlistId, songIds, entryIds, after.entryId(), before.entryId());
    }
    
    void method playlistsChanged()
    {
        // hocus bogus
    }
    
    void method freeUpMemory()
    {
    }
    
    vector<int> method possibleRatings() const
    {
        return ISession::possibleRatings();
    }
    
    bool method saveForOfflinePossible() const
    {
        return true;
    }
}
