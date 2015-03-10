//
//  SpotifySession.cpp
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/13/13.
//
//

#include "SpotifySession.h"
#include "PlaylistCategory.h"
#include "BasicPlaylist.h"
#include "BackgroundExecutor.h"
#include "SpotifyData.h"
#include "NamedImage.h"
#include "IKeychain.h"


#ifndef DISABLE_SPOTIFY

static sp_session *session = nullptr;

// gear for tizen
static const uint8_t kApiKey[] = {
	0x01, 0x98, 0x1E, 0x71, 0xFC, 0x6D, 0xC6, 0x87, 0x48, 0xE1, 0x04, 0x23, 0x2D, 0xD7, 0x41, 0x29,
	0x77, 0xAD, 0x45, 0x99, 0x6B, 0x20, 0x60, 0x3F, 0x8E, 0x45, 0x5B, 0x01, 0x49, 0x41, 0x4A, 0x84,
	0xDA, 0xB9, 0xE4, 0xC1, 0x35, 0xD0, 0xF6, 0xB6, 0xB8, 0xFD, 0x3F, 0x13, 0xA8, 0x12, 0x07, 0xCB,
	0x5E, 0x9D, 0xDD, 0xCA, 0x51, 0xE3, 0xD4, 0xE0, 0xD8, 0xF1, 0xFF, 0x5B, 0xE0, 0xA6, 0x24, 0xFB,
	0x86, 0x93, 0xB6, 0x17, 0x85, 0xDA, 0xF5, 0x50, 0x5C, 0xDB, 0x2C, 0x94, 0x70, 0x73, 0x38, 0xD9,
	0x83, 0x32, 0xF1, 0xF9, 0x69, 0x32, 0x89, 0x49, 0x0F, 0x30, 0x0B, 0x5E, 0x16, 0xC1, 0x40, 0x81,
	0x4C, 0x75, 0xE2, 0x23, 0x6E, 0x12, 0x6C, 0xAF, 0x80, 0x54, 0x0C, 0xD5, 0x63, 0x31, 0x9B, 0xE1,
	0xBF, 0xF0, 0xBB, 0x47, 0xE6, 0xF3, 0x3E, 0xF6, 0x01, 0x0F, 0xEA, 0x9E, 0xF7, 0x8E, 0x80, 0x04,
	0x03, 0xB9, 0xC7, 0xF1, 0xFB, 0x36, 0x60, 0x13, 0x25, 0xC1, 0x8D, 0x60, 0x5E, 0xB0, 0x45, 0xB3,
	0x01, 0xF7, 0x48, 0x86, 0x5A, 0xBD, 0x52, 0x50, 0xF8, 0x3F, 0x61, 0x49, 0x3E, 0x0A, 0x1F, 0xEA,
	0x3C, 0xCD, 0x1A, 0x61, 0x16, 0xA5, 0x89, 0x9C, 0xDF, 0x6F, 0x1B, 0x67, 0xE6, 0xDA, 0xC7, 0x9C,
	0xA7, 0xBC, 0x35, 0x1D, 0x32, 0x51, 0x94, 0xE9, 0xE9, 0x3C, 0x7E, 0x89, 0xEA, 0x8F, 0xD9, 0x61,
	0x1B, 0x21, 0x34, 0xCE, 0x57, 0xDB, 0xE3, 0x1F, 0x9D, 0xD7, 0x11, 0x10, 0x19, 0xE0, 0x86, 0x60,
	0x82, 0x7E, 0xDB, 0xED, 0x70, 0xF0, 0x4C, 0xCA, 0x1C, 0x37, 0xD5, 0xC3, 0x21, 0x03, 0xF1, 0x46,
	0xEA, 0x94, 0x6C, 0x7F, 0xA3, 0xED, 0xDA, 0x83, 0x9B, 0x32, 0xE9, 0x81, 0x35, 0xDB, 0xF9, 0x8D,
	0xC2, 0x69, 0x41, 0x71, 0x8B, 0xEB, 0x9C, 0x83, 0x16, 0x12, 0xA1, 0xA1, 0x16, 0xBF, 0xE0, 0xC4,
	0x56, 0x09, 0xD1, 0xDE, 0x6F, 0xC5, 0x5C, 0xE1, 0xDA, 0x5B, 0xC0, 0xA7, 0xF0, 0x14, 0x1A, 0xE2,
	0xFB, 0xDF, 0x7F, 0x96, 0xE8, 0x08, 0xFD, 0xFB, 0x14, 0xEC, 0x42, 0xDC, 0x28, 0xF5, 0xFD, 0x8F,
	0x92, 0x8D, 0x3B, 0x5E, 0x41, 0xD9, 0xB6, 0x43, 0x65, 0x69, 0x6B, 0xC5, 0x86, 0x10, 0x90, 0x3C,
	0x68, 0x1F, 0x0F, 0x37, 0xE3, 0x03, 0x41, 0x0D, 0x0D, 0xB0, 0x39, 0x41, 0x7F, 0x34, 0xC6, 0x87,
	0xE0,
};

namespace Gear
{
    using std::cout;
    using std::endl;
    
    struct Notifier
    {
    private:
        bool complete;
        condition_variable condition;
        mutex m;
        sp_error err;
        
    public:
        void reset()
        {
            lock_guard<mutex> l(m);
            complete = false;
            err = SP_ERROR_OK;
        }
        
        void wait()
        {
            unique_lock<mutex> l(m);
            while (!complete) {
                condition.wait(l);
            }
        }
        
        void notify(sp_error aErr)
        {
            lock_guard<mutex> l(m);
            err = aErr;
            complete = true;
            condition.notify_all();
        }
        
        void notify()
        {
            notify(SP_ERROR_OK);
        }
        
        sp_error result()
        {
            wait();
            return err;
        }
    };
    
    static Notifier notifier;
    
#define method SpotifySession::
    
    static SpotifySession *_spotifySession = nullptr;
    
    int method music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
    {
        cout << "got frames: " << num_frames << endl;
        if (_spotifySession->_currentData) {
            return _spotifySession->_currentData->deliver(format, frames, num_frames);
        }
        return 0;
    }
    
    static mutex _processMutex;
    static condition_variable _processVariable;
    static bool wakeupSet;
    static std::chrono::time_point<system_clock> wakeup;

    bool method startProcess()
    {
		static SerialExecutor _processExecutor;
		wakeupSet = false;
    	_processExecutor.addTask([]{
			unique_lock<mutex> l(_processMutex);
    		while(true) {
    			if (wakeupSet) {
    				if (_processVariable.wait_until(l, wakeup) == std::cv_status::timeout) {
    					//AppLog("nextTime activating after wait...");
    					wakeupSet = false;
    					notify_main_thread(NULL);
    				}
    			} else {
    				_processVariable.wait(l);
    			}
    		}
    	});
    	return true;
    }

    void method notify_main_thread(sp_session *)
    {
    	static bool value = startProcess();
    	value = false;

        //cout << "notifymain" << endl;
        if (_spotifySession) {
            _spotifySession->_executor.addTask([]{
                int nextTimeout = 0;
                sp_session_process_events(session, &nextTimeout);

            	//sp_playlistcontainer *container = sp_session_playlistcontainer(session);
				//int num = sp_playlistcontainer_num_playlists(container);
                //AppLog("nextTimeout: %d playlistnum: %d", nextTimeout, num);

                if (nextTimeout > 0) {
					lock_guard<mutex> l(_processMutex);
					wakeupSet = true;
					wakeup = system_clock::now() + milliseconds(nextTimeout);
					_processVariable.notify_all();
                }
            });
        }
    }
    
    static void logged_in(sp_session *session, sp_error error)
    {
        notifier.notify(error);
    }

    static void connection_error(sp_session *session, sp_error error)
    {
        notifier.notify(error);
    }

    void method credentials_blob_updated(sp_session *session, const char *cBlob)
    {
    	string blob = cBlob;
    	IApp::instance()->keychain()->save("spotify", _spotifySession->_user, blob);
    }
    
    shared_ptr<SpotifySession> method create(const shared_ptr<IApp> &app)
    {
        auto ret = shared_ptr<SpotifySession>(new SpotifySession(app));
        
        PlaylistCategory all(kAllCategory, kAllTag, true);
        all.setSingularPlaylist(false);
        ret->_categories.push_back(all);
        
        auto &allCategoryLists = ret->_categories.at(0).playlists();
        auto freeSearch = FreeSearchArray::create(ret);
        allCategoryLists.push_back(shared_ptr<IPlaylist>(new BasicPlaylist(ret, "Search", "free", shared_ptr<ISongArray>(freeSearch))));
        
        ret->_favorites = ModifiablePlaylist::create("all", ret);
		ret->_favorites->setName("Favorites");
		ret->_favorites->setOrdered(false);

		allCategoryLists.push_back(ret->_favorites);

		PlaylistCategory playlists = PlaylistCategory(kPlaylistsCategory, kPlaylistsTag);
		ret->_categories.push_back(playlists);

		PlaylistCategory autoPlaylists = PlaylistCategory(u(kAutoPlaylistsCategory), kAutoPlaylistsTag);
		ret->_positives = SongLibraryPlaylist::create(ret);
		ret->_positives->setName("Thumbs Up");
		ret->_positives->setPlaylistId(PredicatePlaylist::thumbsUpPredicate().fingerPrint());
		autoPlaylists.playlists().push_back(ret->_positives);


		ret->_categories.push_back(autoPlaylists);


		//PlaylistCategory autoPlaylists = PlaylistCategory(u(kAutoPlaylistsCategory), kAutoPlaylistsTag);
		//_categories.push_back(autoPlaylists);

        return ret;
    }
    
    // note: very important to use executor() to ensure libspotify thread safety!
    
    method SpotifySession(const shared_ptr<IApp> &app) :
        SongManipulationSession(app),
        _container(NULL),
        _starredPlaylist(NULL)
    {
        _spotifySession = this;
        _executor.addTask([]{
            if (!session) {
                sp_session_config config;
                bzero(&config, sizeof(config));
                config.api_version = SPOTIFY_API_VERSION;
                config.application_key = (const void *)kApiKey;
                config.application_key_size = sizeof(kApiKey);
                //config.ca_certs_filename = "/Users/agiasztalos/RootCA.pem";
                string path = IApp::instance()->dataPath() + "/spotify";
                config.cache_location = path.c_str();
                //config.compress_playlists = true;
                //config.device_id = "mustsetuuid";
                config.dont_save_metadata_for_playlists = false;
                config.initially_unload_playlists = false;
                //config.proxy = ""; // WARNING! this must be set for tizen. The format is protocol://<host>:port (where protocal is http/https/socks4/socks5)
                //config.proxy_username = "";
                //config.proxy_password = "";
                config.settings_location = config.cache_location;
                //config.tracefile = "";
                config.user_agent = "gearmusic-tizen";
                
                sp_session_callbacks callbacks;
                bzero(&callbacks, sizeof(callbacks));
                callbacks.music_delivery = &music_delivery;
                callbacks.notify_main_thread = &notify_main_thread;
                callbacks.connection_error = &connection_error;
                callbacks.logged_in = &logged_in;
                callbacks.credentials_blob_updated = &credentials_blob_updated;
                
                config.callbacks = &callbacks;
                
                sp_error err = sp_session_create(&config, &session);
                //std::cout << "spotify session create " << err << std::endl;
            }
        });
    }
    
    void method fetch()
    {
    	return;

    	//auto pThis = shared_from_this();
		_executor.addTask([]{
			milliseconds w(200);
			sleep_for(w);
			int nextTimeout = 0;
			sp_session_process_events(session, &nextTimeout);
		});
    }

    bool method autoconnectSync()
    {
    	_noMoreFetch = false;
        bool success = false;

        _user = IApp::instance()->keychain()->username("spotify");
        string blob = IApp::instance()->keychain()->pass("spotify");
        if (_user.empty() || blob.empty()) {
        	return false;
        }

        _executor.addTaskAndWait([&]{
            
            notifier.reset();
            sp_error err = sp_session_login(session, _user.c_str(), NULL, true, blob.c_str());

        	//AppLog("spotify user '%s' blob '%s' err: %d", _user.c_str(), blob.c_str(), err);
            success = (err == SP_ERROR_OK);
            //std::cout << "spotify autoconnect: " << success << std::endl;
        });
        
        if (success) {
            sp_error result = notifier.result();
            success = (result == SP_ERROR_OK);
            if (success) {
            	fetch();
            }
            //std::cout << "spotify autoconnecting: " << result << endl;
        }
        
        return success;
    }

    bool method connectSync(const string &user, const string &pass)
    {
    	_noMoreFetch = false;
    	_user = user;
        bool success = false;
        _executor.addTaskAndWait([&]{
            notifier.reset();
            sp_error err = sp_session_login(session, user.c_str(), pass.c_str(), true, NULL);
            
            success = (err == SP_ERROR_OK);
            //std::cout << "spotify connect: " << success << std::endl;
        });
                                 
        if (success) {
            sp_error result = notifier.result();
            success = (result == SP_ERROR_OK);
            //std::cout << "spotify connecting: " << result << endl;

            if (success) {
            	fetch();
            }
			/*_executor.addTaskAndWait([&]{
				// save credentials
				sp_session_logout(session);
				sp_error err = sp_session_login(session, user.c_str(), pass.c_str(), true, NULL);

				success = (err == SP_ERROR_OK);
			});*/
        };

        // AppLog("spotify connected");
        /*
        if (success) {
        	sp_error result = notifier.result();
			success = (result == SP_ERROR_OK);
        }*/

        return success;
    }
    
    void method disconnectSync()
    {
    	auto pThis = shared_from_this();
        _executor.addTask([pThis]{
        	for (auto it = pThis->_spPlaylists.begin() ; it != pThis->_spPlaylists.end() ; ++it) {
        		//sp_playlist_release(it->second);
        	}
        	pThis->_spPlaylists.clear();
        	// better leak than crash
			//sp_playlistcontainer_release(pThis->_container);
			pThis->_container = nullptr;
			//sp_playlist_release(pThis->_starredPlaylist);
			pThis->_starredPlaylist = nullptr;

            sp_session_logout(session);
        });

        IApp::instance()->keychain()->forget("spotify");
    }
    
    shared_ptr<IPlaylist> method libraryPlaylist()
    {
        return _favorites;
    }
    
    void method addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
    {
        for (auto it = songs.begin() ; it != songs.end() ; ++it) {
        	auto &song = *it;
        	song->setRating(5);
        	song->save();
        }
    }
    
    shared_ptr<IPlaybackData> method playbackDataSync(const ISong &song) const
    {
        sp_link *link = sp_link_create_from_string(song.uniqueId().c_str());
        sp_track *track = sp_link_as_track(link);
                                                   
        shared_ptr<IPlaybackData> ret;
        if (track) {
            ret = SpotifyData::create(const_pointer_cast<SpotifySession>(shared_from_this()), track);
        }
        
        sp_link_release(link);
        
        return ret;
    }
    
    
    void method startTrack(const shared_ptr<SpotifyData> &data)
    {
    	{
    		lock_guard<mutex> l(_currentDataMutex);
			if (_currentData == data) {
				return;
			}
    	}

        auto pThis = shared_from_this();
        _executor.addTask([pThis,data]{
            sp_error err = sp_session_player_load(session, data->_track);
            if (err != SP_ERROR_OK) {
            	//AppLog("spotify load %d", err);
            	return;
            }
			//cout << "spotify playa: " << err << endl;
			err = sp_session_player_play(session, true);
			if (err != SP_ERROR_OK) {
            	//AppLog("spotify play %d", err);
            	return;
			}
			//cout << "spotify playb: " << err << endl;

    		lock_guard<mutex> l(pThis->_currentDataMutex);
	        pThis->_currentData = data;
        });
    }
    
    const vector<PlaylistCategory> method categories() const
    {
        return _categories;
    }
    
    static void addSongsToPlaylist(sp_playlist *playlist, const vector<shared_ptr<ISong>> &songs)
    {
    	for (auto it = songs.begin() ; it != songs.end() ; ++it) {
			auto &song = *it;
			auto songId = song->uniqueId();
			sp_link *songLink = sp_link_create_from_string(songId.c_str());
			if (!songLink) {
				continue;
			}
			sp_track *spSong = sp_link_as_track(songLink);
			int num = sp_playlist_num_tracks(playlist);
			if (spSong) {
				sp_error err = sp_playlist_add_tracks(playlist, &spSong, 1, num, session);
				cout << err;
			}
			sp_link_release(songLink);
		}
    }

    vector<SongEntry> method addSongsToPlaylistSync(const string &playlistId, const vector<shared_ptr<ISong>> &songs)
    {
    	vector<SongEntry> entries;
    	auto itPlaylist = _spPlaylists.find(playlistId);
    	if (itPlaylist == _spPlaylists.end()) {
    		return entries;
    	}
    	sp_playlist *playlist = itPlaylist->second;
    	if (playlist) {
    		addSongsToPlaylist(playlist, songs);
    	}

    	for (auto it = _userPlaylists.begin() ; it != _userPlaylists.end() ; ++it) {
    		auto &playlist = *it;
    		if (playlistId == playlist->playlistId()) {
    			transform(songs.begin(), songs.end(), back_inserter(entries),[](const shared_ptr<ISong> &song){
    				return SongEntry(song, song->uniqueId());
    			});
    			playlist->addSongsLocally(entries);
    			_favorites->addSongsLocally(entries);
    			break;
    		}
    	}

        return entries;
    }

    void method createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name)
    {
    	auto pThis = shared_from_this();
		_executor.addTask([songs, name, pThis]{
			if (!pThis->_container) {
				return;
			}

			sp_playlist *playlist = sp_playlistcontainer_add_new_playlist(pThis->_container, name.c_str());
			sp_playlist_add_ref(playlist);
			sp_link *link = sp_link_create_from_playlist(playlist);
			string uniqueId;
			if (link) {
				char buf[4096];
				sp_link_as_string(link, buf, sizeof(buf));
				uniqueId = buf;
				sp_link_release(link);
			}
			pThis->_spPlaylists.insert(make_pair(uniqueId, playlist));
			addSongsToPlaylist(playlist, songs);

			shared_ptr<ModifiablePlaylist> newPlaylist = ModifiablePlaylist::create(uniqueId, pThis);

			vector<SongEntry> entries;
			transform(songs.begin(), songs.end(), back_inserter(entries), [](const shared_ptr<ISong> &rhs){
				return SongEntry(rhs, rhs->uniqueId());
			});
			newPlaylist->addSongsLocally(entries);
			newPlaylist->setName(name);
			pThis->_userPlaylists.push_back(newPlaylist);
			auto &playlists = pThis->categoryLocalByTag(kPlaylistsTag).playlists();
			playlists.push_back(newPlaylist);

			pThis->_playlistsChangeSignal.signal();
		});
    }
    
    static void deleteFromPlaylist(sp_playlist *playlist, const vector<string> &songIds, bool clearStarred = false)
    {
    	for (auto it = songIds.begin() ; it != songIds.end() ; ++it) {
			sp_link *songLink = sp_link_create_from_string(it->c_str());
			if (!songLink) {
				continue;
			}
			sp_track *spSong = sp_link_as_track(songLink);
			if (spSong) {
				if (clearStarred) {
					sp_track_set_starred(session, &spSong, 1, false);
				}

				int num = sp_playlist_num_tracks(playlist);
				for (int i = 0 ; i < num ; ++i) {
					sp_track *track = sp_playlist_track(playlist, i);
					if (track == spSong) {
						sp_error err = sp_playlist_remove_tracks(playlist, &i, 1);
						break;
					}
				}
				//cout << err;
			}
			sp_link_release(songLink);
		}
    }

	void method deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId)
	{
		if (playlistId == "all") {
			bool clear = true;
			for (auto it = _spPlaylists.begin() ; it != _spPlaylists.end() ; ++it) {
				deleteFromPlaylist(it->second, songIds, clear);
				clear = false;
			}

			return;
		}

		auto itPlaylist = _spPlaylists.find(playlistId);
		if (itPlaylist == _spPlaylists.end()) {
			return;
		}
		sp_playlist *playlist = itPlaylist->second;
		if (playlist) {
			deleteFromPlaylist(playlist, songIds);
		}
	}

    void method removePlaylist(const shared_ptr<IPlaylist> &playlist)
    {
        
    }
    
    void method changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before)
    {
        
    }
    
    void method changePlaylistNameSync(const string &newName, const string &playlistId)
    {
        
    }
    
    vector<shared_ptr<IPlaylist>> method playlists()
    {
    	vector<shared_ptr<IPlaylist>> ret;
		ret.reserve(_userPlaylists.size());
		transform(_userPlaylists.begin(), _userPlaylists.end(), back_inserter(ret), [](const shared_ptr<Gear::ModifiablePlaylist> &rhs){
			return rhs;
		});
		return ret;
    }

    static void search_complete(sp_search *result, void *userData)
    {
        notifier.notify();
    }
    
    shared_ptr<ISong> method songForTrack(sp_track *track)
    {
    	//AppLog("spotsearch track 1");
    	if (!track) {
			return shared_ptr<ISong>();
    	}

    	auto availability = sp_track_get_availability(session, track);
    	if (availability != SP_TRACK_AVAILABILITY_AVAILABLE) {
			return shared_ptr<ISong>();
		}

    	//AppLog("spotsearch track 2");

        char buf[4096];

		sp_link *link = sp_link_create_from_track(track, 0);
    	//AppLog("spotsearch track 3");
		if (!link) {
			return shared_ptr<ISong>();
		}
    	//AppLog("spotsearch track 4");
		sp_link_as_string(link, buf, sizeof(buf));
		string uniqueId = buf;
		sp_link_release(link);
    	//AppLog("spotsearch track 5");

		function<shared_ptr<const ISong>(const shared_ptr<const ISong> &)> f = [&](const shared_ptr<const ISong> &original)->shared_ptr<const ISong>{
			if (original) {
				//shared_ptr<ISong> ret = original;
				return original;
			}

			shared_ptr<ModifiableSong> song(new ModifiableSong(uniqueId, this->shared_from_this()));
			song->setUintForKey("rating", sp_track_is_starred(session, track) ? 5 : 0);
			//AppLog("spotsearch track 6");
			song->setStringForKey("title", sp_track_name(track));
			//AppLog("spotsearch track 7");
			if (sp_track_num_artists(track) > 0) {
				//AppLog("spotsearch track 8");
				song->setStringForKey("artist", sp_artist_name(sp_track_artist(track, 0)));
			}
			//AppLog("spotsearch track 9");
			sp_album *album = sp_track_album(track);
			if (album) {
				//AppLog("spotsearch track 10 %x", album);
				song->setStringForKey("album", sp_album_name(album));
				//AppLog("spotsearch track 11 %x", album);
				sp_link *coverArt = sp_link_create_from_album_cover(album, SP_IMAGE_SIZE_SMALL);
				if (coverArt) {
					//AppLog("spotsearch track 12 %x", coverArt);
					sp_link_as_string(coverArt, buf, sizeof(buf));
					//AppLog("spotsearch track 13 %x", coverArt);
					string coverArtLink = buf;
					string prefix = "spotify:image:";
					if (coverArtLink.size() >= prefix.size()) {
						coverArtLink = "http://o.scdn.co/140/" + coverArtLink.substr(prefix.size());
						song->setStringForKey("albumArtUrl", coverArtLink);
					}
					sp_link_release(coverArt);
				}
			}

			song->setUintForKey("durationMillis", sp_track_duration(track));
			//AppLog("spotsearch track 14");
			song->setUintForKey("track", sp_track_index(track));
			//AppLog("spotsearch track 15");
			song->setUintForKey("disc", sp_track_disc(track));
			//AppLog("spotsearch track 16");
			return shared_ptr<const ISong>(song);
		};

        assert(false); // out of order
		//shared_ptr<const ISong> ret = StashTransaction<shared_ptr<const ISong>>::put(uniqueId, f);

		// temporary hack
		return const_pointer_cast<ISong>(ret);
    }

    const vector<SongEntry> method searchSync(const string &filter, string &token)
    {
    	_noMoreFetch = true;
    	static const int kChunkSize = 50;

    	int offset = 0;
        if (token.empty()) {
            token = "0";
        } else {
        	{
				std::stringstream ss;
				ss << token;
				ss >> offset;
				if (offset > 100) {
					return vector<SongEntry>();
				}
        	}
        	offset += kChunkSize;
        	std::stringstream ss;
        	ss << offset;
        	token = ss.str();
        }
        
        sp_search *search;
        
        notifier.reset();
        
        //AppLog("spotsearch 1 '%s'", filter.c_str());

        _executor.addTaskAndWait([&]{
			AppLog("spotsearch 2 '%s' %d", filter.c_str(), offset);
            search = sp_search_create(session, filter.c_str(), offset, kChunkSize, 0,0,0,0,0,0, SP_SEARCH_STANDARD, &search_complete, NULL);
			//AppLog("spotsearch 3 '%s'", filter.c_str());
        });
        
		//AppLog("spotsearch 4 '%s'", filter.c_str());
        notifier.wait();
		//AppLog("spotsearch 5 '%s'", filter.c_str());
        
        vector<SongEntry> ret;
        auto pThis = shared_from_this();
        
        _executor.addTaskAndWait([&]{
			//AppLog("spotsearch 6 '%s'", filter.c_str());
            int num = sp_search_num_tracks(search);
			AppLog("spotsearch 7 '%s' %d", filter.c_str(), num);
            for (int i = 0 ; i < num ; i++) {
				//AppLog("spotsearch 8 '%s' %d/%d", filter.c_str(), i, num);
                sp_track *track = sp_search_track(search, i);
				//AppLog("spotsearch 9 '%s' %d/%d %x", filter.c_str(), i, num, track);
                
                auto song = pThis->songForTrack(track);
				//AppLog("spotsearch 10 '%s' %d/%d %x", filter.c_str(), i, num, track);
                if (song) {
                    ret.push_back(SongEntry(song, song->uniqueId()));
                }
            }
            
            sp_search_release(search);
			//AppLog("spotsearch 11 '%s' %d", filter.c_str(), num);
        });
        
        int relevance = offset;
        for (auto it = ret.begin() ; it != ret.end() ; ++it) {
			it->song()->setUintForKey("relevance", relevance);
			++relevance;
		}

        return ret;
    }

    void method seekTrack(float seconds)
    {
    	auto pThis = shared_from_this();
		_executor.addTask([pThis, seconds]{
			sp_error err = sp_session_player_seek(session, seconds * 1000);
			cout << "seek: " << err << endl;
		});
    }

    shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<ISong> &song) const
	{
    	return shared_ptr<Gui::IPaintable>(new Gui::NamedImage("favicon-spotify"));
	}

	shared_ptr<Gui::IPaintable> method sessionIcon(const shared_ptr<IPlaylist> &playlist) const
	{
		return shared_ptr<Gui::IPaintable>(new Gui::NamedImage("favicon-spotify"));
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

	void method doRefresh()
	{
		_refreshing = true;

		fetchPlaylists();

		_refreshing = false;
	}

	vector<SongEntry> method fetchPlaylist(sp_playlist *playlist)
	{
		vector<SongEntry> ret;
		if (!playlist) {
			return ret;
		}

		bool isLoaded = sp_playlist_is_loaded(playlist);
		if (!isLoaded) {
			//AppLog("not loaded :(");
		}

		int num = sp_playlist_num_tracks(playlist);
		for (int i = 0 ; i < num ; ++i) {
			sp_track *track = sp_playlist_track(playlist, i);
			if (!track) {
				continue;
			}
			auto song = songForTrack(track);
			if (song) {
				ret.push_back(SongEntry(song, song->uniqueId()));
			}
		}

		return ret;
	}

	void method playlist_metadata_updated(sp_playlist *playlist, void *userdata)
	{
		if (_spotifySession->_noMoreFetch) {
			return;
		}
		string name(sp_playlist_name(playlist));
		int num = sp_playlist_num_tracks(playlist);
		//std::cout << "title: " << name;

		_spotifySession->_executor.addTask([]{
			_spotifySession->fetchPlaylists();
		});
	}

	/*

	static void playlist_renamed(sp_playlist *playlist, void *userdata)
	{
		string name(sp_playlist_name(playlist));
		std::cout << "title: " << name;
	}

	static void playlist_state_changed(sp_playlist *playlist, void *userdata)
	{
		string name(sp_playlist_name(playlist));
		std::cout << "title: " << name;
	}

	static void playlist_added(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
	{
		string name(sp_playlist_name(playlist));
		std::cout << "title: " << name;
	}

	static void container_loaded(sp_playlistcontainer *pc, void *userdata)
	{
		int num = sp_playlistcontainer_num_playlists(pc);
		std::cout << "title: " << num;
	}*/

	void method fetchPlaylists()
	{
		if (_starredPlaylist) {
			sp_playlist_release(_starredPlaylist);
		}
		_starredPlaylist = sp_session_starred_create(session);
		sp_playlist_callbacks starredCallbacks;
		bzero(&starredCallbacks, sizeof(starredCallbacks));
		starredCallbacks.playlist_metadata_updated = playlist_metadata_updated;
		sp_playlist_add_callbacks(_starredPlaylist, &starredCallbacks, NULL);

		auto entries = fetchPlaylist(_starredPlaylist);

		_favorites->addSongsLocally(entries);
		_positives->addSongsLocally(entries);

		// TODO
		if (_container) {
			//sp_playlistcontainer_release(_container);
		}
		_container = sp_session_playlistcontainer(session);
		//_container = sp_session_publishedcontainer_for_user_create(session, NULL);

		/*sp_playlist_callbacks callbacks;
		bzero(&callbacks, sizeof(callbacks));
		callbacks.playlist_metadata_updated = playlist_metadata_updated;
		callbacks.playlist_renamed = playlist_renamed;
		callbacks.playlist_state_changed = playlist_state_changed;
*/
		int num = sp_playlistcontainer_num_playlists(_container);
		/*sp_playlistcontainer_callbacks containerCallbacks;
		bzero(&containerCallbacks, sizeof(containerCallbacks));
		containerCallbacks.playlist_added = playlist_added;
		containerCallbacks.container_loaded = container_loaded;
		sp_playlistcontainer_add_callbacks(_container, &containerCallbacks, NULL);
*/
		for (auto it = _spPlaylists.begin() ; it != _spPlaylists.end() ; ++it) {
			sp_playlist *playlist = it->second;
			sp_playlist_release(playlist);
		}
		_userPlaylists.clear();
		auto &playlists = categoryLocalByTag(kPlaylistsTag).playlists();
		playlists.clear();

		for (int i = 0 ; i < num ; ++i) {
			sp_playlist *playlist = sp_playlistcontainer_playlist(_container, i);
			//sp_playlist_set_in_ram(session, playlist, true);
			//sp_playlist_add_callbacks(playlist, &callbacks, NULL);
			auto entries = fetchPlaylist(playlist);

			// duplicates are skipped
			_favorites->addSongsLocally(entries);

			string uniqueId;
			string name(sp_playlist_name(playlist));
			sp_link *link = sp_link_create_from_playlist(playlist);
			if (link) {
				char buf[4096];
				sp_link_as_string(link, buf, sizeof(buf));
				uniqueId = buf;
				sp_link_release(link);
			} else {
				uniqueId = name;
			}
			sp_playlist_add_ref(playlist);
			_spPlaylists.insert(make_pair(uniqueId, playlist));
			shared_ptr<ModifiablePlaylist> newPlaylist = ModifiablePlaylist::create(uniqueId, shared_from_this());

			newPlaylist->addSongsLocally(entries);
			newPlaylist->setName(name);
			_userPlaylists.push_back(newPlaylist);
			playlists.push_back(newPlaylist);
		}

		//sp_playlistcontainer_release(container);

		_playlistsChangeSignal.signal();
	}

	void method modifySongs(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts)
	{
		auto pThis = shared_from_this();
		_executor.addTask([pThis, changedInts, ids]{

			auto ratingPresent = changedInts.find("rating");
			if (ratingPresent != changedInts.end()) {
				long long rating = ratingPresent->second;

				for (auto it = ids.begin() ; it != ids.end() ; ++it) {

					sp_link *link = sp_link_create_from_string(it->c_str());
					if (link) {
						sp_track *track = sp_link_as_track(link);
						int nextTimeout = 0;
						sp_session_process_events(session, &nextTimeout);
						BackgroundExecutor::instance().addTask([pThis, track, rating,link]{
							milliseconds w(1000);
							sleep_for(w);
							// wait for the track data to arrive in a slightly hackish way...
							pThis->_executor.addTask([pThis, track, rating,link]{
								if (track) {
									if (rating == 5) {
										sp_track_set_starred(session, &track, 1, true);
										auto song = pThis->songForTrack(track);
										if (song) {
											vector<SongEntry> songs;
											songs.push_back(SongEntry(song, song->uniqueId()));
											pThis->_favorites->addSongsLocally(songs);
											pThis->_positives->addSongsLocally(songs);
										}
									} else {
										bool found = false;
										sp_track_set_starred(session, &track, 1, false);
										auto song = pThis->songForTrack(track);
										if (song) {
											SongEntry entry(song, song->uniqueId());
											vector<SongEntry> songs;
											songs.push_back(entry);
											pThis->_positives->removeSongs(songs);
											for (auto itPlaylist = pThis->_userPlaylists.begin() ; itPlaylist != pThis->_userPlaylists.end() ; ++itPlaylist) {
												auto &playlist = *itPlaylist;
												if (playlist->songArray()->contains(entry)) {
													found = true;
													break;
												}
											}

											if (!found) {
												pThis->_favorites->removeSongs(songs);
											}
										}
									}
								}
								sp_link_release(link);
							});
						});
					}
				}
			}
		});
	}

    vector<int> method possibleRatings() const
	{
    	vector<int> ret;
    	ret.push_back(0);
    	ret.push_back(5);
    	return ret;
	}
}

#endif
