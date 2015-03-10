//
//  GooglePlaySession.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#ifndef __G_Ear__GooglePlaySession__
#define __G_Ear__GooglePlaySession__

#include "stdplus.h"
#include SHAREDFROMTHIS_H
#include "json-forwards.h"
#include "DbSession.h"
#include "SerialExecutor.h"
#include "GuardedData.h"

namespace Gear
{
    using std::deque;
    using namespace Base;
    
    class ModifiablePlaylist;
    class StoredSongArray;
    class SongLibraryPlaylist;
    class IArtist;
    class IAlbum;
    template<class T>
    class IConceiverFactory;
    class GooglePlayThumbsPlaylist;
    class GooglePlaySessionImpl;
    
    class GooglePlaySession : public DbSession
    {
    public:
        static shared_ptr<GooglePlaySession> create(const shared_ptr<IApp> &app);
        
        ~GooglePlaySession();
        
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs);
        //virtual const vector<Json::Value> radioSongsSync(const string &radioId);
        //const vector<Json::Value> sharedSongsSync(const string &playlistId);
        //const vector<Json::Value> ratedSongsSync();
        virtual ValidPtr<const vector<PlaylistCategory>>categories() const;
        
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before);
        void addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result);
        
        virtual void createPlaylist(const vector<string> &songIds, const string &name, const function<void(const shared_ptr<Json::Value> &)> &callback) = 0;
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId) = 0;
        
        
        bool allAccessCapable() const;
        // will be private
        void setAllAccessCapable(bool allAccessCapable);
        
        //const shared_ptr<IPlaylist> createRadioSync(shared_ptr<ISong> &song, const string &fieldHint);
        SerialExecutor &executor();
        
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId) = 0;
        virtual void modifySongs(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts);

        
        virtual void freeUpMemory();
        void storeOffline();
        virtual void dispose();
        virtual vector<int> possibleRatings() const;
        virtual bool saveForOfflinePossible() const;

        static const std::string sourceIdentifier;

    protected:
        // since we need to be managed by a shared_ptr, this mustn't be constructed directly
        GooglePlaySession(const shared_ptr<const IConceiverFactory<GooglePlaySession>> &conceiverFactory, const shared_ptr<IApp> &app);
        
        
        static bool isAllAccessId(const string &str);
        void playlistsChanged();
    
        shared_ptr<SongLibraryPlaylist> _libraryPlaylist;
        
    protected:
        const shared_ptr<const IConceiverFactory<GooglePlaySession>> _conceiverFactory;
        
    private:
        GooglePlaySession(const GooglePlaySession &rhs); // delete
        const GooglePlaySession &operator=(const GooglePlaySession &rhs); // delete
        
        static DbSession::Behaviour behaviour(GooglePlaySession *nself);
        shared_ptr<GooglePlaySessionImpl> shared_from_this();

        SerialExecutor _executor;
        SerialExecutor _prioritizedExecutor;
        
        void search(const string &filter, const Json::Value &context, const DbSession::Behaviour::FetchSearchCallback &callback);

        virtual void searchAllAccess(const string &filter, const function<void(const shared_ptr<Json::Value> &)> &callback) = 0;
        virtual void fetchArtist(const string &artistId, const function<void(const shared_ptr<Json::Value> &)> &callback) = 0;
        virtual void fetchAlbum(const string &albumId, const function<void(const shared_ptr<Json::Value> &)> &callback) = 0;
        
        //const vector<Json::Value> searchSync(const string &filter, string &token, bool &finished);
        virtual void fetchRadioFeed(const string &radioId, const function<void(const shared_ptr<Json::Value> &)> &f) const = 0;
        
        virtual void addAllAccessIdsToLibrary(const vector<string> &songs, const function<void(const shared_ptr<Json::Value> &)> &f) = 0;
        virtual void radioList(const function<void(const vector<Json::Value> &)> &results);
        virtual void loadRadio(const function<void(const shared_ptr<Json::Value> &)> &f) const = 0;
        virtual void createRadio(const string &songId, const string &name, const int type, const function<void(const shared_ptr<Json::Value> &)> &f) const = 0;
        virtual void removeRadio(const string &radioId) = 0;
        virtual void removePlaylist(const string &playlistId) = 0;

        virtual void addSongsToPlaylist(const string &playlistId, const vector<string> &songIds, const function<void(const shared_ptr<Json::Value> &)> &callback) = 0;

        virtual void fetchSharedSongs(const string &playlistId, const function<void(const shared_ptr<Json::Value> &)> &f) const = 0;
        virtual void fetchRatedSongs(const function<void(const shared_ptr<Json::Value> &)> &f) const = 0;
        
        void fetchRadios();
        virtual void modifySongsSync(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts) = 0;
            virtual void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const std::function<void(const shared_ptr<IPlaylist> &)> &result);
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<string> &songIds, const vector<string> &entryIds, const string &afterId, const string &before) = 0;
        
        virtual void connect(const string &user, const string &keychainData, const function<void(bool)> &callback) = 0;
        
        virtual string userName() const = 0;

        static vector<SongEntry> mixSongsAndPlaylistIds(const vector<shared_ptr<ISong>> &songs, const vector<Json::Value> entriesForIds);
        virtual shared_ptr<IPlaybackData> playbackDataSync(const string &uniqueId) const = 0;


        string lastToken;
        deque<shared_ptr<IArtist>> artistsToFetch;
        deque<shared_ptr<IAlbum>> albumsToFetch;
        int _relevance;
        
        
        bool _allAccessCapable;
        
        shared_ptr<GooglePlayThumbsPlaylist> _thumbsUpEphemeral;
        
        friend class GooglePlayConceiver;
        friend class GooglePlaySessionImpl;
    };
}

#endif /* defined(__G_Ear__GooglePlaySession__) */
