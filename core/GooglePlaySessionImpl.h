//
//  GooglePlaySessionImpl.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#ifndef __G_Ear__GooglePlaySessionImpl__
#define __G_Ear__GooglePlaySessionImpl__

#include "GooglePlaySession.h"
#include "GooglePlayConceiver.h"
#include ATOMIC_H

namespace Base
{
    class ManagedObject;
}

namespace Gear
{
    class GoogleMusicProtocol;
    
    class GooglePlaySessionImpl : public GooglePlaySession
    {
    public:
        static shared_ptr<GooglePlaySessionImpl> create(const shared_ptr<IApp> &app);
        
        virtual shared_ptr<IPlaybackData> playbackDataSync(const ISong &song) const;
        virtual bool connectSync(const string &user, const string &keychainData);
        
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<ISong> &song) const;
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<IPlaylist> &song) const;

    protected:
        GooglePlaySessionImpl(const shared_ptr<IApp> &app);
        void init();

    private:
        virtual void loadFromCache();
        virtual void checkAllAccess();
        virtual const shared_ptr<Json::Value> searchAllAccess(const string &filter);
        virtual const shared_ptr<Json::Value> fetchArtist(const string &artistId);
        virtual const shared_ptr<Json::Value> fetchAlbum(const string &albumId);
        virtual shared_ptr<Json::Value> addAllAccessIdsToLibrarySync(const vector<string> &songs);
        virtual void modifySongsSync(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts);

        virtual const shared_ptr<Json::Value> loadRadio() const;
        virtual const shared_ptr<Json::Value> fetchRadioFeed(const string &radioId) const;
        virtual const shared_ptr<Json::Value> fetchSharedSongs(const string &playlistId) const;
        virtual const shared_ptr<Json::Value> fetchRatedSongs() const;
        virtual const shared_ptr<Json::Value> createRadio(const string &songId, const string &name, const int type) const;
        virtual void removeRadio(const string &radioId);
        virtual void fetchLibrary();
        virtual void fetchPlaylists();
        void addToPlaylist(const shared_ptr<IPlaylist> localPlaylist, vector<shared_ptr<ManagedObject>> &managedObjects);
        //vector<SongEntry> addToPlaylist(const Json::Value &songs, const shared_ptr<IPlaylist> destination);
        vector<SongEntry> addToPlaylistAfterConceive(const Json::Value &songs, GooglePlayConceiver::Hint hint, const shared_ptr<IPlaylist> destination);
        
        virtual const shared_ptr<Json::Value>  addSongsToPlaylistSync(const string &playlistId, const vector<string> &songIds);
        virtual const shared_ptr<Json::Value> createPlaylistSync(const vector<string> &strongIds, const string &name);
        virtual void removePlaylist(const string &playlistId);
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<string> &songIds, const vector<string> &entryIds, const string &afterId, const string &before);
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId);
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId);
        virtual string userName() const;
        
        THREAD_NS::mutex _cacheFetchMutex;
        THREAD_NS::condition_variable _cacheFetchCondition;
        bool _cacheFetched;
        
        shared_ptr<GoogleMusicProtocol> _protocol;
    };
}

#endif /* defined(__G_Ear__GooglePlaySessionImpl__) */
