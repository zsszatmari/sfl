//
//  SpotifySession.h
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/13/13.
//
//

#ifndef __G_Ear_Player__SpotifySession__
#define __G_Ear_Player__SpotifySession__

#include "SongManipulationSession.h"
#include "SpotifyService.h"
#ifndef DISABLE_SPOTIFY
#include <libspotify/api.h>

namespace Gear
{
    class SpotifyData;
    class ModifiablePlaylist;
    class SongLibraryPlaylist;
    
    class SpotifySession final : public SongManipulationSession, public MEMORY_NS::enable_shared_from_this<SpotifySession>
    {
    public:
        static shared_ptr<SpotifySession> create(const shared_ptr<IApp> &app);
        
        bool autoconnectSync();
        bool connectSync(const string &user, const string &pass);
        void disconnectSync();
        
        virtual shared_ptr<IPlaylist> libraryPlaylist();
        virtual void refresh();
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs);
        virtual shared_ptr<IPlaybackData> playbackDataSync(const ISong &song) const;
        virtual const vector<PlaylistCategory> categories() const;
        
        virtual void modifySongs(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts);
        virtual vector<SongEntry> addSongsToPlaylistSync(const string &playlistId, const vector<shared_ptr<ISong>> &songs);
        virtual void removePlaylist(const shared_ptr<IPlaylist> &playlist);
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId);
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before);
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId);
        
        virtual const vector<SongEntry> searchSync(const string &filter, string &token);
        
        virtual vector<shared_ptr<IPlaylist>> playlists();
        void startTrack(const shared_ptr<SpotifyData> &data);
        void seekTrack(float seconds);
        virtual vector<int> possibleRatings() const;
        
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<ISong> &song) const;
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<IPlaylist> &playlist) const;

    private:
        virtual void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name);
        void doRefresh();
        void fetch();
        void fetchPlaylists();
        shared_ptr<ISong> songForTrack(sp_track *track);
        vector<SongEntry> fetchPlaylist(sp_playlist *playlist);
        
        SpotifySession(const shared_ptr<IApp> &app);
        
        shared_ptr<ModifiablePlaylist> _favorites;
        shared_ptr<SongLibraryPlaylist> _positives;
        vector<shared_ptr<ModifiablePlaylist>> _userPlaylists;
        map<string, sp_playlist *> _spPlaylists;

        static bool startProcess();
        static void notify_main_thread(sp_session *session);
        static int music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames);
        static void credentials_blob_updated(sp_session *session, const char *cBlob);
        static void playlist_metadata_updated(sp_playlist *playlist, void *userdata);

        
        atomic_bool _noMoreFetch;
        shared_ptr<SpotifyData> _currentData;
        mutex _currentDataMutex;
        string _user;

        sp_playlistcontainer *_container;
        sp_playlist *_starredPlaylist;
    };
}
#endif

#endif /* defined(__G_Ear_Player__SpotifySession__) */
