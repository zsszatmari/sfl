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
#include "ModifiablePlaylist.h"
#include "IKeychain.h"
#include "GooglePlayService.h"
#include "RemoteGooglePlaylist.h"
#include "SongLibraryPlaylist.h"
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
#include "Db.h"
#include "IoService.h"

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
        shared_ptr<GooglePlaySessionImpl> ret = MEMORY_NS::static_pointer_cast<GooglePlaySessionImpl>(shared_from_this());
        
        _protocol = shared_ptr<GoogleMusicProtocol>(new GoogleMusicProtocol(GoogleMusicConnection::create()));
    }
    
    bool method connectSync(const string &user, const string &keychainData)
    {
        assert(Io::isCurrent());
        
        auto protocol = _protocol;
        auto pThis = shared_from_this();
        auto &connectedSignal = _connectedSignal;
        
        bool success = false;
            
        Json::Reader reader;
        Json::Value cookies;
        if (reader.parse(keychainData, cookies)) {
            protocol->setUserName(user);
        }
        success = protocol->loginWithCookies(cookies);                        
    
        checkAllAccess();
        // must do this here, because we need info on whether all access is present
        createSpecial();
        
        if (success) {
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
            
            IApp::instance()->keychain()->save(GooglePlayService::identifier(), protocol->userName(), encodedCookies);
            
            // because pThis is also retained, this is guaranteed to survive
            connectedSignal.signal();
            
            pThis->initialFetchSync();
            
        }
        return success;
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
    
    /*void method addToPlaylist(const shared_ptr<IPlaylist> localPlaylist, vector<shared_ptr<ManagedObject>> &all)
    {
        vector<SongEntry> converted;
        converted.reserve(all.size());
        
        auto destinationPlaylist = dynamic_pointer_cast<ModifiablePlaylist>(localPlaylist);
        auto sharedSession = shared_from_this();
        
        {
            ModifiableSongContext context;

            transform(all.begin(), all.end(), back_inserter(converted), [sharedSession, destinationPlaylist, &context](const shared_ptr<ManagedObject> &song){
                
                shared_ptr<ModifiableSong> librarySong = context.create(song, sharedSession);
                
                if (destinationPlaylist) {
                    // playlistEntryId is deprecated, should not use...
                    const string entryId = librarySong->baaadstringForKey("playlistEntryId");
                    return SongEntry(librarySong, entryId);
                } else {
                    return SongEntry(librarySong);
                }
            });
        }
            
        auto destinationLibrary = dynamic_pointer_cast<SongLibraryPlaylist>(localPlaylist);
        if (destinationLibrary) {
            destinationLibrary->addSongsLocally(converted);
        } else if (destinationPlaylist) {
            destinationPlaylist->setSongsLocally(converted);
        }
    }
    */
    void method loadFromCache()
    {
        // not necessary, everyhing should be immediately available
        // ... but must work on that part for the playlists... 
        /*auto pLibrary = libraryPlaylist();
        auto sharedSession = shared_from_this();
        auto songEntity = _songEntity;
        auto playlistEntity = _playlistEntity;
        auto &document = _document;
        
        function<void()> f = [pLibrary, sharedSession, songEntity, playlistEntity, &document] () mutable {
            
            auto pImpl = reinterpret_cast<GooglePlaySessionImpl *>(sharedSession.get());
            
            {
                Base::DbPredicate predicate("playlist", "all");
                auto songs = document->fetch(songEntity, predicate, sharedSession);
                
                pImpl->addToPlaylist(pLibrary, songs);
            }            
            
            auto playlistObjects = document->fetch(playlistEntity, DbPredicate(), sharedSession);
            
            for (auto it = playlistObjects.begin() ; it != playlistObjects.end() ; ++it) {
            	auto &playlistObject = *it;

                shared_ptr<ModifiablePlaylist> list = shared_ptr<ModifiablePlaylist>(ModifiablePlaylist::create(playlistObject, sharedSession));
                //cout << "stored list: " << utf16ToUtf8(list->name()) << endl;
                const string playlistId = playlistObject->valueForKey("id").getString();
                
                pImpl->_data.access([playlistId, list](GooglePlaySession::Data &data){
                    auto &localPlaylists = data._playlistsById;
                    
                    localPlaylists.insert(make_pair(playlistId, list));
                    
                    return 0;
                });
                
                
                Base::DbPredicate predicate("playlist", list->playlistId());
                auto songs = document->fetch(songEntity, predicate, sharedSession);
                                            
                pImpl->addToPlaylist(list, songs);
            }
        };
    
        f();
        {
            lock_guard<mutex> l(_cacheFetchMutex);
            _cacheFetched = true;
            _cacheFetchCondition.notify_all();
        }
        
        playlistsChanged();
        */
    }
    
    static shared_ptr<Json::Value> sanitize(const shared_ptr<Json::Value> &value)
    {
        if (!value) {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        return value;
    }
    
    void method checkAllAccess()
    {
        auto ret = _protocol->loadExplore();
        
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
    }
    
    const shared_ptr<Json::Value> method searchAllAccess(const string &filter)
    {
        return sanitize(_protocol->searchSongs(filter));
    }
    
    shared_ptr<IPlaybackData> method playbackDataSync(const ISong &song) const
    {
        const string uniqueId = song.uniqueId();
        if (OfflineStorage::instance().available(uniqueId)) {
            return OfflineStorage::instance().fetch(uniqueId, IPlaybackData::Format::Mp3);
        }
        
        if (isAllAccessId(uniqueId)) {
            
            return _protocol->playDirect(uniqueId);
        } else {
            return _protocol->play(uniqueId);
        }
    }
    
    const shared_ptr<Json::Value> method fetchArtist(const string &artistId)
    {
        return sanitize(_protocol->fetchArtist(artistId));
    }
    
    const shared_ptr<Json::Value> method fetchAlbum(const string &albumId)
    {
        return sanitize(_protocol->fetchAlbum(albumId));
    }
    
    shared_ptr<Json::Value> method addAllAccessIdsToLibrarySync(const vector<string> &songs)
    {
        shared_ptr<Json::Value> json = _protocol->addSongsToLocker(songs);
        
        return sanitize(json);
    }
    
    const shared_ptr<Json::Value> method fetchRadioFeed(const string &radioId) const
    {
        return sanitize(_protocol->fetchRadioFeed(radioId));
    }
    
    const shared_ptr<Json::Value> method fetchSharedSongs(const string &playlistId) const
    {
        return sanitize(_protocol->fetchSharedSongs(playlistId));
    }
    
    const shared_ptr<Json::Value> method fetchRatedSongs() const
    {
        return sanitize(_protocol->fetchRatedSongs());
    }
    
    const shared_ptr<Json::Value> method loadRadio() const
    {
        return sanitize(_protocol->loadRadio());
    }
    
    const shared_ptr<Json::Value> method createRadio(const string &songId, const string &name, const int type) const
    {
        return sanitize(_protocol->createStation(songId, name, type));
    }
    
    void method removeRadio(const string &radioId)
    {
        _protocol->deleteStation(radioId);
    }
    
    void method removePlaylist(const string &playlistId)
    {
        _protocol->deletePlaylist(playlistId);
    }
    
    vector<SongEntry> method addToPlaylistAfterConceive(const Json::Value &json, GooglePlayConceiver::Hint hint, const shared_ptr<IPlaylist> destination)
    {
        auto pThis = shared_from_this();
        auto conceiver = _conceiverFactory->conceiver(json, pThis, static_cast<int>(hint));
        
        if (!destination) {
            return vector<SongEntry>();
        }
        
        string playlistId = destination->playlistId();
        
        auto librarySongs = conceiver->songs(playlistId);
        if (conceiver->starRatedPresent()) {
            auto &prefs = IApp::instance()->preferences();
            string autoKey = "FiveStarRatingsAuto";

            if (!prefs.boolForKey(autoKey)) {
                prefs.setUintForKey(autoKey, true);
                prefs.setUintForKey("FiveStarRatings", true);
            }
        }
        
        auto destinationLibrary = MEMORY_NS::dynamic_pointer_cast<SongLibraryPlaylist>(destination);
        auto destinationPlaylist = MEMORY_NS::dynamic_pointer_cast<ModifiablePlaylist>(destination);
        
        /*if (destinationLibrary) {
            destinationLibrary->addSongsLocally(librarySongs);
        } else if (destinationPlaylist) {
            destinationPlaylist->setSongsLocally(librarySongs);
        }*/
        return librarySongs;
    }
    
    void method fetchLibrary()
    {
        assert(Io::isCurrent());

        auto pLibrary = libraryPlaylist();
        
        vector<string> toRemove = pLibrary->songArray()->all("entryId");
        auto originalSize = toRemove.size();
        sort(toRemove.begin(), toRemove.end());
        
        auto pThis = MEMORY_NS::static_pointer_cast<GooglePlaySessionImpl>(shared_from_this());
        auto gotDictionaries = [&](const Json::Value &aSongs){
            
            if (aSongs.size() < 1) {
                return;
            }
            
            pThis->executor().addTaskAndWait([&]{
                auto &songs = aSongs[0u];
                
#ifdef DEBUG
                std::cout << "got songs: " << songs.size() << std::endl;
#endif

                //Json::StyledWriter debWriter;
                //std::cout << "2 " << debWriter.write(aSongs) << std::endl;
                //std::cout << "3 " << debWriter.write(songs) << std::endl;
                                
                vector<string> justAdded = sfl::map(
                    [](const SongEntry &entry){return entry.entryId();},
                    pThis->addToPlaylistAfterConceive(songs, GooglePlayConceiver::Hint::loadLibrary, pLibrary));
                sort(justAdded.begin(), justAdded.end());
                // The difference of two sets is formed by the elements that are present in the first set, but not in the second one. The elements copied by the function come always from the first range, in the same order.
                std::vector<std::string> diff;
                diff.reserve(toRemove.size());
                set_difference(toRemove.begin(), toRemove.end(), justAdded.begin(), justAdded.end(), back_inserter(diff));
                swap(toRemove, diff);
            });
        };
        
        _protocol->getAllSongs(gotDictionaries);

#pragma message ("TODO: do similar wipe of old items with youtube as well, and possibly with google music playlist contents")
        IApp::instance()->db()->remove("Song", 
                                       pThis->_libraryPlaylist->songArray()->impl().inherentPredicate(),
                                       "entryId",
                                       toRemove);

        Tracking::track("Google Music", "Number of Songs", originalSize - toRemove.size());
            
#ifdef DEBUG
        std::cout << "library fetched and finalized\n";
#endif
    }
    
    void method fetchPlaylists()
    {
#ifdef DEBUG_DISABLE_REMOTEFETCHSONGS
        return;
#endif
        shared_ptr<Json::Value> response = _protocol->getListOfPlaylists();
        if (!response) {
#ifdef DEBUG
            std::cout << "playlists: none\n";
#endif
            return;
        }
        size_t size = response->size();
#ifdef DEBUG
        std::cout << "playlists: " << size << "\n";
#endif
        
        std::map<string, shared_ptr<IPlaylist>> allLists;
        for (int i = 0 ; i < size ; ++i) {
            auto &playlistItem = (*response)[i];
            /* 
             [["ccfded4d-de4b-4ff8-be62-d317a16a0507","classical",1368115209096046,1385029520526000,1,"AMaBXylGpdXkDTCHHOpY1NcSw9FKp6IGKV7bVbZvNYHc8VP1CLnuliJwC4NaA1TLBF3BQeFkZ012VRQPUH7QFGHAPwRHcGuakQ\u003d\u003d",0,"","Zsolt Szatm\xc3\xa1ri",0\n,"http://lh3.googleusercontent.com/-Ree4f3Pdejo/AAAAAAAAAAI/AAAAAAAAZws/NYTTjNo4bLA/photo.jpg"]\n,
             */
            
            if (playlistItem.size() < 5) {
                continue;
            }
            string playlistId = playlistItem[0u].asString();        
            string name = decode_html_entities_utf8(playlistItem[1u].asString());
            
            if (playlistId.empty()) {
                // some kind of google db failure
                continue;
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
                    continue;
                }
            } else {
            
                pSongs = _protocol->getPlaylist(playlistId);
                if (!pSongs || !pSongs->isArray()) {
                    continue;
                }
            }
            
            auto pThis = MEMORY_NS::static_pointer_cast<GooglePlaySessionImpl>(shared_from_this());
            
            _data.accessAndWait([playlistId, isShared, pThis, name, token, pSongs, &allLists](GooglePlaySession::Data &data){
                
                auto &localPlaylists = data._playlistsById;
                auto it = localPlaylists.find(playlistId);
                shared_ptr<IPlaylist> list;
                
                if (!isShared) {
                    if (it != localPlaylists.end()) {
                        list = it->second;
                        
                        allLists.insert(*it);
                    } else {
                        
                        list = ModifiablePlaylist::create(playlistId, name, pThis);
                        
                        // allLists must be maintained in and retrieved from db...
                        auto p = make_pair(playlistId, list);
                        allLists.insert(p);
                        localPlaylists.insert(p);
                    }
                    
                    auto stored = MEMORY_NS::dynamic_pointer_cast<ModifiablePlaylist>(list);
                    if (stored) {
                        stored->setNameLocally(name);
                        stored->setToken(token);
                    }
                    
                    // pSongs validaed before
                    auto &songs = *pSongs;
                    pThis->addToPlaylistAfterConceive(songs, GooglePlayConceiver::Hint::loadPlaylist, list);
                    
                } else {
                    if (it != localPlaylists.end()) {
                        localPlaylists.erase(it);
                    }
                    
                    list = shared_ptr<RemoteGooglePlaylist>(RemoteGooglePlaylist::create(u(name), playlistId, token, pThis));
                    auto p = make_pair(playlistId, list);
                    localPlaylists.insert(p);
                    allLists.insert(p);
                    
                    // this does not make sense, there are no songs in this case
                    //pThis->addToPlaylistAfterConceive(songs, GooglePlayConceiver::Hint::loadPlaylist, list);
                }
                // ?????
                //pThis->addToPlaylistAfterConceive(songs, GooglePlayConceiver::Hint(), list);
                
                return 0;
            });

        }
        
        // drop unused
        _data.accessAndWait([&allLists](GooglePlaySession::Data &data){
            data._playlistsById = allLists;
            return 0;
        });
        
        playlistsChanged();
    }
    
    const shared_ptr<Json::Value> method addSongsToPlaylistSync(const string &playlistId, const vector<string> &songIds)
    {
        return _protocol->addPlaylistEntries(songIds, playlistId);
    }
    
    const shared_ptr<Json::Value> method createPlaylistSync(const vector<string> &songIds, const string &name)
    {
        return sanitize(_protocol->createPlaylist(songIds, name));
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

    shared_ptr<IPaintable> method sessionIcon(const shared_ptr<ISong> &song) const
	{
    	return shared_ptr<IPaintable>(new NamedImage("favicon-gplay"));
	}

    shared_ptr<IPaintable> method sessionIcon(const shared_ptr<IPlaylist> &song) const
	{
		return shared_ptr<IPaintable>(new NamedImage("favicon-gplay"));
	}
}
