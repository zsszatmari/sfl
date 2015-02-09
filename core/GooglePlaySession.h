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
    class GooglePlayRadio;
    class SongLibraryPlaylist;
    class IArtist;
    class IAlbum;
    template<class T>
    class IConceiverFactory;
    class GooglePlayThumbsPlaylist;
    
    class GooglePlaySession : public DbSession, public MEMORY_NS::enable_shared_from_this<GooglePlaySession>
    {
    public:
        static shared_ptr<GooglePlaySession> create(const shared_ptr<IApp> &app);
        
        ~GooglePlaySession();
        virtual const vector<SongEntry> searchSync(const string &filter, string &token);
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs);
        virtual const vector<SongEntry> radioSongsSync(const string &radioId);
        const vector<SongEntry> sharedSongsSync(const string &playlistId);
        const vector<SongEntry> ratedSongsSync();
        virtual void initialFetchSync();
        virtual ValidPtr<const vector<PlaylistCategory>>categories() const;
        void removeRadio(const GooglePlayRadio &radio);
        virtual void removePlaylist(const shared_ptr<IPlaylist> &playlist);
        shared_ptr<StoredSongArray> librarySongArray();
        virtual vector<shared_ptr<IPlaylist>> playlists();
        virtual shared_ptr<IPlaylist> libraryPlaylist();
        
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<SongEntry> &songs, const SongEntry &afterId, const SongEntry &before);
        virtual void addSongsToPlaylistAsync(const string &playlistId, const vector<shared_ptr<ISong>> &songs, const function<void(const vector<SongEntry> &)> &result);
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId) = 0;
        virtual void refresh();
        
        
        bool allAccessCapable() const;
        // will be private
        void setAllAccessCapable(bool allAccessCapable);
        
        const shared_ptr<IPlaylist> createRadioSync(shared_ptr<ISong> &song, const string &fieldHint);
        SerialExecutor &executor();
        
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId) = 0;
        virtual void modifySongs(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts);
        
        virtual bool connectSync(const string &user, const string &keychainData) = 0;
        
        class Data
        {
        public:
            std::map<string, shared_ptr<IPlaylist>> _playlistsById;
            Data();

        private:
            Data(const Data &); // delete
            Data &operator=(const Data &); // delete
         };
        
        virtual void freeUpMemory();
        void storeOffline();
        virtual void dispose();
        virtual vector<int> possibleRatings() const;
        virtual bool saveForOfflinePossible() const;

        static const std::string sourceIdentifier;
        
    protected:
        // since we need to be managed by a shared_ptr, this mustn't be constructed directly
        GooglePlaySession(const shared_ptr<const IConceiverFactory<GooglePlaySession>> &conceiverFactory, const shared_ptr<IApp> &app);
        
        Base::GuardedData<Data> _data;
        
        
        void createSpecial();
        static bool isAllAccessId(const string &str);
        void playlistsChanged();
    
        shared_ptr<SongLibraryPlaylist> _libraryPlaylist;
        
    protected:
        const shared_ptr<const IConceiverFactory<GooglePlaySession>> _conceiverFactory;
        
    private:
        GooglePlaySession(const GooglePlaySession &rhs); // delete
        const GooglePlaySession &operator=(const GooglePlaySession &rhs); // delete
        

        SerialExecutor _executor;
        SerialExecutor _prioritizedExecutor;
        
        virtual const shared_ptr<Json::Value> searchAllAccess(const string &filter) = 0;
        virtual const shared_ptr<Json::Value> fetchArtist(const string &artistId) = 0;
        virtual const shared_ptr<Json::Value> fetchAlbum(const string &albumId) = 0;
        const vector<SongEntry> searchSync(const string &filter, string &token, bool &finished);
        virtual shared_ptr<Json::Value> addAllAccessIdsToLibrarySync(const vector<string> &songs) = 0;
        const vector<shared_ptr<IPlaylist>> radioList();
        virtual void loadFromCache() = 0;
        virtual const shared_ptr<Json::Value> loadRadio() const = 0;
        virtual const shared_ptr<Json::Value> createRadio(const string &songId, const string &name, const int type) const = 0;
        virtual void removeRadio(const string &radioId) = 0;
        virtual void removePlaylist(const string &playlistId) = 0;
        virtual const shared_ptr<Json::Value> fetchRadioFeed(const string &radioId) const = 0;
        virtual const shared_ptr<Json::Value> fetchSharedSongs(const string &radioId) const = 0;
        virtual const shared_ptr<Json::Value> fetchRatedSongs() const = 0;
        virtual void checkAllAccess() = 0;
        virtual void fetchLibrary() = 0;
        virtual void fetchPlaylists() = 0;
        void fetchRadios();
        virtual void modifySongsSync(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts) = 0;
        virtual const shared_ptr<Json::Value> addSongsToPlaylistSync(const string &playlistId, const vector<string> &songIds) = 0;
        virtual const shared_ptr<Json::Value> createPlaylistSync(const vector<string> &strongIds, const string &name) = 0;
        virtual void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const std::function<void(const shared_ptr<IPlaylist> &)> &result);
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<string> &songIds, const vector<string> &entryIds, const string &afterId, const string &before) = 0;
        
        virtual string userName() const = 0;

        static vector<SongEntry> mixSongsAndPlaylistIds(const vector<shared_ptr<ISong>> &songs, const vector<SongEntry> entriesForIds);

        string lastToken;
        deque<shared_ptr<IArtist>> artistsToFetch;
        deque<shared_ptr<IAlbum>> albumsToFetch;
        int _relevance;
        
        void doRefresh();
        
        bool _allAccessCapable;
        
        shared_ptr<GooglePlayThumbsPlaylist> _thumbsUpEphemeral;
        
        friend class GooglePlayConceiver;
    };
}

#endif /* defined(__G_Ear__GooglePlaySession__) */
