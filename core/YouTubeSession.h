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
#include "DbSession.h"
#include "SerialExecutor.h"
#include "AtomicPtr.h"
#include "YouTubeSessionBase.h"

namespace Gear
{
    class OAuthConnection;
    class SongLibraryPlaylist;
    class ModifiablePlaylist;

    using std::function;
    using std::pair;
    
    class YouTubeSession final : public DbSession
    {
    public:
        static DbSession::Behaviour createBehaviour(YouTubeSession *nself);

        static shared_ptr<YouTubeSession> create(const shared_ptr<IApp> &app);
        static shared_ptr<YouTubeSession> activeSession();
        
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs);
        //virtual void fetchRatings(const vector<SongEntry> &entries);
        
        virtual ValidPtr<const vector<PlaylistCategory>> categories() const;
        
        //virtual void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result);
        virtual void modifySongs(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts);
        
        virtual void addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result);
        virtual void removePlaylist(const string &playlist);
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId);
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before);
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId);
        virtual vector<int> possibleRatings() const;
        
    private:
        YouTubeSession(const shared_ptr<IApp> &app);
        
        virtual shared_ptr<YouTubeSession> loggedInSession();
        
        
        Base::SerialExecutor _executor;
        Base::SerialExecutor _outOfOrder;
        shared_ptr<YouTubeSession> shared_from_this();
        
        void tryWithUserChannel(const function<void(const function<void()> &)> &fn, const function<void()> &aFail);

        void addSongsToPlaylistAsync(const shared_ptr<ModifiablePlaylist> &playlist, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result);
        void addSongsToPlaylistAsync(const shared_ptr<ModifiablePlaylist> &playlist, const vector<shared_ptr<ISong>> &songs, const string &playlistId, const function<void(const vector<SongEntry> &)> &result);
        void fetchPositive(const function<void(const vector<Json::Value> &)> &result);

        void fetchPlaylists(const DbSession::Behaviour::FetchCallback &callback, const function<void()> &finished);
        void fetchPlaylistsForCategory(const string &category, const vector<pair<string,string>> &idsAndNames, const function<void(const string &, const string &,const vector<Json::Value> &)> &gotData, const function<void()> &finished);
        void fetchSubscriptions(const vector<pair<string,string>> &channelIdsAndTitles, const function<void(const string &, const string &,const vector<Json::Value> &)> &gotData, const function<void()> &finished);
        
        void fetchSubscriptions(const DbSession::Behaviour::FetchCallback &callback, const function<void()> &finished);
        shared_ptr<ModifiablePlaylist> fetchPlaylist(const string &playlistId, const string &name, bool ordered = false);
        void fetchPlaylistIdsAndNames(const string &callUrl, const string &titlePrefix, const function<void(const vector<pair<string,string>> &)> &f);
        void fetchPlaylistItems(const string &playlistId, const function<void(const vector<Json::Value> &)> &result);

        //string callSync(const string &str, const string &m, const string &body, int *resultCode = nullptr);
        void addUserPlaylistLocally(const shared_ptr<ModifiablePlaylist> playlist);
        
        shared_ptr<OAuthConnection> _connection;
        
        string _favoritesId;

        DbSession::Behaviour behaviour(YouTubeSession *nself);

        void resetConnection();

        YouTubeSessionBase::CallAsync callAsync();
        void callAsync(const std::string &request, const std::string &m, const std::string &body, const std::map<string,string> &headers, const function<void(const string &, int)> &result);

    };
}

#endif /* defined(__G_Ear_Player__YouTubeSession__) */
