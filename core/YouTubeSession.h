//
//  YouTubeSession.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/4/13.
//
//

#ifndef __G_Ear_Player__YouTubeSession__
#define __G_Ear_Player__YouTubeSession__

#include "stdplus.h"
#include "YouTubeSessionBase.h"
#include "SerialExecutor.h"
#include "AtomicPtr.h"

namespace Gear
{
    class OAuthConnection;
    class SongLibraryPlaylist;
    class ModifiablePlaylist;
    using std::function;
    
    class YouTubeSession final : public YouTubeSessionBase {
    public:
        
        static shared_ptr<YouTubeSession> create(const shared_ptr<IApp> &app, const shared_ptr<OAuthConnection> &connection);
        static shared_ptr<YouTubeSession> activeSession();
        
        void connect();
        virtual vector<shared_ptr<IPlaylist>> playlists();
        
        virtual shared_ptr<IPlaylist> libraryPlaylist();
        virtual void refresh();
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs);
        virtual void fetchRatings(const vector<SongEntry> &entries);
        
        virtual ValidPtr<const vector<PlaylistCategory>> categories() const;
        
        virtual void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result);
        virtual void modifySongs(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts);
        
        virtual void addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result);
        virtual void removePlaylist(const shared_ptr<IPlaylist> &playlist);
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId);
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before);
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId);
        virtual vector<int> possibleRatings() const;
        
    private:
        YouTubeSession(const shared_ptr<IApp> &app, const shared_ptr<OAuthConnection> &connection);
        
        virtual shared_ptr<YouTubeSession> loggedInSession();
        void createCategories();
        void doAddToPlaylist(const shared_ptr<ModifiablePlaylist> &playlist, const shared_ptr<ISong> &song, const function<void(const vector<SongEntry> &)> &result);
        void doAddToPlaylist(const shared_ptr<ModifiablePlaylist> &playlist, const shared_ptr<ISong> &song, const string &playlistId, const function<void(const vector<SongEntry> &)> &result);
        
        
        Base::SerialExecutor _executor;
        Base::SerialExecutor _outOfOrder;
        void doRefresh();
        shared_ptr<YouTubeSession> shared_from_this();
        
        void tryWithUserChannel(const function<void(const function<void()> &)> &fn, const function<void()> &aFail);

        void addSongsToPlaylistAsync(const shared_ptr<ModifiablePlaylist> &playlist, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result);
        void addSongsToPlaylistAsync(const shared_ptr<ModifiablePlaylist> &playlist, const vector<shared_ptr<ISong>> &songs, const string &playlistId, const function<void(const vector<SongEntry> &)> &result);
        vector<SongEntry> fetchPositive();
        void fetchPlaylists();
        shared_ptr<ModifiablePlaylist> fetchPlaylist(const string &playlistId, const string &name, bool ordered = false);
        std::vector<std::pair<string,string>> fetchPlaylistIdsAndNames(const string &callUrl, const string &prefix);
        vector<SongEntry> fetchPlaylistItems(const string &playlistId);
        virtual string callSync(const string &str);
        string callSync(const string &str, const string &m, const string &body, int *resultCode = nullptr);
        void addUserPlaylistLocally(const shared_ptr<ModifiablePlaylist> playlist);
        
        shared_ptr<OAuthConnection> _connection;
        AtomicPtr<vector<shared_ptr<ModifiablePlaylist>>> _userPlaylists;
        
        shared_ptr<ModifiablePlaylist> _favorites;
        shared_ptr<SongLibraryPlaylist> _positives;
        
        string _favoritesId;
    };
}

#endif /* defined(__G_Ear_Player__YouTubeSession__) */
