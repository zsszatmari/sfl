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
#include "GooglePlayRadio.h"
#include "MainExecutor.h"
#include "ModifiablePlaylist.h"
#include "SongGoogleLibraryPlaylist.h"
#include "BaseUtility.h"
#include "Color.h"
#include "AllAccessPlaylist.h"
#include "PredicatePlaylist.h"
#include "MusicSortCompare.h"
#include "GooglePlayThumbsPlaylist.h"
#include "UnionPlaylist.h"
#include "CoreDebug.h"
#include "Tracking.h"
#include "sfl/Prelude.h"
#include "sfl/Vector.h"

namespace Gear
{
    using std::function;
    using std::map;

    const std::string GooglePlaySession::sourceIdentifier = "gplay";


    GooglePlaySession::Data::Data()
    {
    }
    
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

    static DbSession::Behaviour behaviour()
    {
        DbSession::Behaviour behaviour;
        behaviour.sourceIdentifier = GooglePlaySession::sourceIdentifier;
        return behaviour;
    }

    method GooglePlaySession(const shared_ptr<const IConceiverFactory<GooglePlaySession>> &conceiverFactory, const shared_ptr<IApp> &app) :
        DbSession(app, behaviour()),
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
    
    void method createSpecial()
    {
        bool allAccess = allAccessCapable();

        _libraryPlaylist = SongGoogleLibraryPlaylist::create(shared_from_this());
        modifyCategoryByTag(kAllTag, [&](PlaylistCategory &allCategory){
            
            auto &allCategoryLists = allCategory.playlists();
            allCategoryLists.clear();
            if (allAccess) {
                auto allAccessList = shared_ptr<IPlaylist>(new AllAccessPlaylist(shared_from_this()));
                allCategoryLists.push_back(allAccessList);
            }
            Tracking::track("Google Music", "All Access", allAccess);
            allCategoryLists.push_back(_libraryPlaylist);
        });
        modifyCategoryByTag(kRadioTag, [&](PlaylistCategory &radioCategory){
            if (allAccess) {
                
                radioCategory.setTitle("Radio");
            } else {
                radioCategory.setTitle("Instant Mix");
            }
        });
        modifyCategoryByTag(kAutoPlaylistsTag, [&](PlaylistCategory &category){
            auto &autoLists = category.playlists();
            autoLists.clear();
            
            shared_ptr<IPlaylist> thumbsUpLocal(PredicatePlaylist::thumbsUpPlaylist(librarySongArray()));
            
            _thumbsUpEphemeral = shared_ptr<GooglePlayThumbsPlaylist>(new GooglePlayThumbsPlaylist(shared_from_this()));
            shared_ptr<UnionPlaylist> unionThumbsUp(new UnionPlaylist(PredicatePlaylist::thumbsUpPredicate().fingerPrint(), thumbsUpLocal->name()));
            shared_ptr<IPlaylist> t[] = {thumbsUpLocal, _thumbsUpEphemeral};
            unionThumbsUp->setPlaylists(init<vector<shared_ptr<IPlaylist>>>(t));
            
            autoLists.push_back(unionThumbsUp);
            //#warning houstooon
            //autoLists.push_back(thumbsUpLocal);
            

            // these are taken care of in sessionmanager
            //autoLists.push_back(PredicatePlaylist::thumbsDownPlaylist(librarySongArray()));
#ifndef TIZEN
            //autoLists.push_back(PredicatePlaylist::unratedPlaylist(librarySongArray()));
            //autoLists.push_back(PredicatePlaylist::recentlyAddedPlaylist(librarySongArray()));
#endif
        });
        
        playlistsChanged();
    }
    
    bool method allAccessCapable() const
    {
        return _allAccessCapable;
    }
    
    void method setAllAccessCapable(bool allAccessCapable)
    {
        _allAccessCapable = allAccessCapable;
    }

    const vector<SongEntry> method searchSync(const string &filter, string &token)
    {
        bool finished = false;
        while (true) {
            auto ret = searchSync(filter, token, finished);
            if (!ret.empty() || finished) {
                return ret;
            }
        }
    }

    const vector<SongEntry> method searchSync(const string &filter, string &token, bool &finished)
    {
        finished = false;
        
        vector<SongEntry> ret;
        auto &rel = _relevance;
        auto updateRelevance = [&ret, &rel]{
            for (auto it = ret.begin() ; it != ret.end() ; ++it) {
                it->song()->setUintForKey("relevance", rel);
            }
        };
        
        
        if (token.empty()) {
            
            _relevance = 0;
            string newToken;
            do {
                newToken = random_string(10);
            } while (newToken == lastToken);
            lastToken = newToken;
            token = newToken;
            
            auto json = searchAllAccess(filter);
            if (!json) {
                return vector<SongEntry>();
            }
            auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this());
            ret = conceiver->songs("temp:free");
            
            auto artistList = conceiver->artists();
            auto albumList = conceiver->albums();
            
            artistsToFetch = deque<shared_ptr<IArtist>>(artistList.begin(), artistList.end());
            albumsToFetch = deque<shared_ptr<IAlbum>>(albumList.begin(), albumList.end());
            
            return ret;
        } else {
            ++_relevance;
            
            if (!artistsToFetch.empty()) {
                auto artist = artistsToFetch.front();
                artistsToFetch.pop_front();
                
                auto json = fetchArtist(artist->uniqueId());
                auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this(),static_cast<int>(GooglePlayConceiver::Hint::fetchArtist));
                ret = conceiver->songs("temp:free");
                auto additionalAlbums = conceiver->albums();
                // avoid duplicates
                auto eraseIt = remove_if(additionalAlbums.begin(), additionalAlbums.end(), [&](const shared_ptr<IAlbum> &item){

                	for (auto it = albumsToFetch.begin() ; it != albumsToFetch.end() ; ++it) {
                		auto &rhs = *it;

                		if (equals(rhs, item)) {
                            return true;
                        }
                    }
                    return false;
                });
                additionalAlbums.erase(eraseIt, additionalAlbums.end());
                copy(additionalAlbums.begin(), additionalAlbums.end(), back_inserter(albumsToFetch));
                
                updateRelevance();
                
                return ret;
            } else if (!albumsToFetch.empty()) {
                auto album = albumsToFetch.front();
                albumsToFetch.pop_front();
                
                auto json = fetchAlbum(album->uniqueId());
                auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this(), static_cast<int>(GooglePlayConceiver::Hint::fetchAlbum));
                ret = conceiver->songs("temp:free");
                
                updateRelevance();
                
                return ret;
            } else {
                finished = true;
                return vector<SongEntry>();
            }
        }
    }
    
    const vector<SongEntry> method radioSongsSync(const string &radioId)
    {
        auto json = fetchRadioFeed(radioId);
        auto pThis = shared_from_this();
        
        auto conceiver = _conceiverFactory->conceiver(*json, pThis);
        return conceiver->songs("temp:" + radioId);
    }
    
    const vector<SongEntry> method sharedSongsSync(const string &playlistId)
    {
        auto json = fetchSharedSongs(playlistId);
        auto pThis = shared_from_this();
        auto conceiver = _conceiverFactory->conceiver(*json, pThis);
        return conceiver->songs(playlistId);
    }
    
    const vector<SongEntry> method ratedSongsSync()
    {
        auto json = fetchRatedSongs();
        auto pThis = shared_from_this();
        auto conceiver = _conceiverFactory->conceiver(*json, pThis);
        // not temporary! otherwise we couldn't merge
        return conceiver->songs("ephemthumbsup");   
    }
                            
    bool method isAllAccessId(const string &str)
    {
        return str.find("-") == string::npos;
    }
    
    void method addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
    {
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
            _prioritizedExecutor.addTask([pThis, songIds, songs]{
                shared_ptr<Json::Value> json = pThis->addAllAccessIdsToLibrarySync(songIds);
                auto conceiver = pThis->_conceiverFactory->conceiver(*json, pThis, static_cast<int>(GooglePlayConceiver::Hint::addToLibrary));
                auto gotSongs = conceiver->songs("all");
                
                vector<SongEntry> newSongs;
                
                for (auto itEntry = gotSongs.begin() ; itEntry != gotSongs.end() ; ++itEntry) {
                	auto &gotEntry = *itEntry;

                	auto allAccessId = gotEntry.song()->uniqueId();
                    auto libraryId = gotEntry.entryId();
                    
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
                
                pThis->_libraryPlaylist->addSongsLocally(newSongs);
            });
        }
    }
    
    ValidPtr<const vector<PlaylistCategory>> method categories() const
    {
        return _categories;
    }
    
    void method fetchRadios()
    {
        // fetch radios
        auto radio = radioList();
        
        modifyCategoryByTag(kRadioTag, [&](PlaylistCategory &category){
            category.playlists() = radio;
        });
        _playlistsChangeSignal.signal();
    }
    
    void method initialFetchSync()
    {
        _refreshing = true;
        
        #pragma message("TODO: delete legacy db")
        /*if (app()) {
            str << app()->dataPath() << "/";
        }
        str << "googlemusic-" << userName() << ".db";
        */
        
        doRefresh();
    }
    
    void method dispose()
    {
    }
    
    void method doRefresh()
    {
        // this may be good, but certainly not cross-platform
#ifdef __APPLE__
        setpriority(PRIO_DARWIN_THREAD, 0, PRIO_DARWIN_BG);
#endif
        
        // not here, it's too late
        //checkAllAccess();
        
        
        
        _refreshing = true;
        
        loadFromCache();
    
        // fetch library
        fetchRadios();
        fetchLibrary();
#ifdef DEBUG
        std::cout << "library fetched\n";
#endif

#ifndef DEBUG_DISABLE_REMOTEFETCHPLAYLISTS
        fetchPlaylists();
#endif
#ifdef DEBUG
        std::cout << "playlists fetched\n";
#endif

        _refreshing = false;
    }
    
    void method refresh()
    {
        if (_refreshing) {
            return;
        }
        
        auto pThis = shared_from_this();
        
        _executor.addTask([pThis]{
        
            pThis->doRefresh();
        });
    }
    
    const vector<shared_ptr<IPlaylist>> method radioList()
    {
        auto json = loadRadio();
        auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this(), static_cast<int>(GooglePlayConceiver::Hint::radioListUser));
        
        auto ret = conceiver->playlists();
        shared_ptr<GooglePlayRadio> feelinglucky = GooglePlayRadio::create("I'm Feeling Lucky", "", shared_from_this());
        feelinglucky->setEditable(false);
        ret.insert(ret.begin(), feelinglucky);
        
        return ret;
    }
    
    SerialExecutor & method executor()
    {
        return _executor;
    }
    
    void method modifySongs(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts)
    {
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
        e.addTask(f);
    }
    
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
            newPlaylist->setName(radioName);
            
            modifyCategoryByTag(kRadioTag, [&](PlaylistCategory &category) {
                
                vector<shared_ptr<IPlaylist>> &playlists = category.playlists();
                auto foundOrAdd = [&]()->std::pair<vector<shared_ptr<IPlaylist>>, shared_ptr<IPlaylist>> {
                    return match<std::pair<vector<shared_ptr<IPlaylist>>,shared_ptr<IPlaylist>>>(find([&](const shared_ptr<IPlaylist> &p){
                        //std::cout << "comparing " << p->playlistId() << "(" << p->name() << ")" <<  " with " << useId << std::endl;
                        return p->playlistId() == newPlaylist->playlistId();
                    }, playlists),
                                 [&](const Nothing &){

                        return std::make_pair(snoc(playlists, newPlaylist),newPlaylist);
                    }, [&](const shared_ptr<IPlaylist> &found){
                        return std::make_pair(playlists, found);});
                    }();
                category.playlists() = foundOrAdd.first;
                newPlaylist = foundOrAdd.second;          
            });
    
            playlistsChanged();
            
            return newPlaylist;
        }
        return shared_ptr<IPlaylist>();
    }
    
    void method removeRadio(const GooglePlayRadio &radio)
    {
        modifyCategoryByTag(kRadioTag, [&](PlaylistCategory &category){
            auto &radios = category.playlists();
            vector<shared_ptr<IPlaylist>>::iterator radioIt;
            //cout << "removing " << radio.playlistId() << endl;
            for (auto radioIt = radios.begin() ; radioIt != radios.end() ; ++radioIt) {
                //cout << "checking " << radioIt->get()->playlistId() << endl;
                
                if (*radioIt->get() == radio) {
                    radios.erase(radioIt);
                    break;
                }
            }
        });
        
        auto pThis = shared_from_this();
        auto uniqueId = radio.radioId();
        _executor.addTask([uniqueId, pThis]{
            pThis->removeRadio(uniqueId);
        });
        
        playlistsChanged();
    }
    
    void method removePlaylist(const shared_ptr<IPlaylist> &playlist)
    {
        const auto uniqueId = playlist->playlistId();
        auto pThis = shared_from_this();
        
        _data.access([uniqueId, pThis](GooglePlaySession::Data &data){
            auto it = data._playlistsById.find(uniqueId);
            if (it != data._playlistsById.end()) {
                data._playlistsById.erase(it);
                pThis->_executor.addTask([uniqueId, pThis]{
                    pThis->removePlaylist(uniqueId);
                });
            }
            pThis->playlistsChanged();
            return 0;
        });
    }
    
    shared_ptr<StoredSongArray> method librarySongArray()
    {
        if (_libraryPlaylist) {
            return _libraryPlaylist->storedSongArray();
        }
        return shared_ptr<StoredSongArray>();
    }
    
    shared_ptr<IPlaylist> method libraryPlaylist()
    {
        return _libraryPlaylist;
    }
    
    
    vector<shared_ptr<IPlaylist>> method playlists()
    {
        return readCategoryByTag(kPlaylistsTag).playlists();
    }
    
    static const vector<string> songIdsFromSongs(const vector<shared_ptr<ISong>> &songs)
    {
        vector<string> songIds;
        songIds.reserve(songs.size());
        transform(songs.begin(), songs.end(), back_inserter(songIds), [](const shared_ptr<ISong> &rhs){
            return rhs->uniqueId();
        });
        return songIds;
    }
    
    vector<SongEntry> method mixSongsAndPlaylistIds(const vector<shared_ptr<ISong>> &songs, const vector<SongEntry> entriesForIds)
    {
        if (songs.size() != entriesForIds.size()) {
            return vector<SongEntry>();
        }
        
        vector<SongEntry> newEntries;
        newEntries.reserve(songs.size());
        
        auto itId = entriesForIds.begin();
        transform(songs.begin(), songs.end(), back_inserter(newEntries), [&itId](const shared_ptr<ISong> &song) {
            
            SongEntry entry(song, itId->entryId());
            ++itId;
            return entry;
        });
        
        return newEntries;
    }
    
    void method addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result)
    {        
        vector<string> songIds = songIdsFromSongs(songs);
        
#ifdef DEBUG
        std::cout << "TODO: GooglePlaySession::addSongsToPlaylistAsync() is blocking\n";
#endif
        auto json = addSongsToPlaylistSync(playlistId, songIds);
        if (!json) {
            result(vector<SongEntry>());
            return;
        }
        auto conceiver = _conceiverFactory->conceiver(*json, shared_from_this(), static_cast<int>(GooglePlayConceiver::Hint::addToPlaylist));
        
        result(mixSongsAndPlaylistIds(songs, conceiver->songs(playlistId)));
    }
    
    
    
    void method createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result)
    {        
        auto pThis = shared_from_this();
        auto &conceiverFactory = _conceiverFactory;
        
        
        executor().addTask([pThis, this, songs, &conceiverFactory, name, result] () {
            
            vector<string> songIds = songIdsFromSongs(songs);
            shared_ptr<Json::Value> json = pThis->createPlaylistSync(songIds, name);
            auto conceiver = conceiverFactory->conceiver(*json, pThis, static_cast<int>(GooglePlayConceiver::Hint::createPlaylist));
            auto playlists = conceiver->playlists();
            if (playlists.size() < 1) {
                result(shared_ptr<IPlaylist>());
                return;
            }
            auto playlistForId = playlists.at(0);
            
            auto playlistId = playlistForId->playlistId();
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
                
                pThis->_data.accessAndWait([&](GooglePlaySession::Data &data){
                    
                    auto &playlistMap = data._playlistsById;
                    playlistMap.insert(p);
                    return 0;
                });
                pThis->app()->editPlaylistName(playlist);
                pThis->playlistsChanged();
                
                result(playlist);
            });
        });
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
        //categoryLocalByTag(kPlaylistsTag).playlists() = playlists();
        auto pThis = shared_from_this();
        _data.access([pThis](const GooglePlaySession::Data &data){
            
            vector<shared_ptr<IPlaylist>> sortedPlaylists;
                
            sortedPlaylists.reserve(data._playlistsById.size());
            for (auto itp = data._playlistsById.begin() ; itp != data._playlistsById.end() ; ++itp) {
            	auto &p = *itp;
            	sortedPlaylists.push_back(p.second);
            }
            
            sort(sortedPlaylists.begin(), sortedPlaylists.end(), [](const shared_ptr<IPlaylist> &lhs, const shared_ptr<IPlaylist> &rhs){
                
                return MusicSortCompare::lowerThan(lhs->name(), rhs->name());
            });
            pThis->modifyCategoryByTag(kPlaylistsTag, [&](PlaylistCategory &category){
                category.playlists() = sortedPlaylists;
            });
            
            pThis->_playlistsChangeSignal.signal();
            return 0;
        });
    }
    
    void method freeUpMemory()
    {
        auto array = librarySongArray();
        if (array) {
            array->freeUpMemory();
        }
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
