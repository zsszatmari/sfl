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
        
        virtual void connect(const string &user, const string &keychainData, const function<void(bool)> &callback);
        
    protected:
        GooglePlaySessionImpl(const shared_ptr<IApp> &app);
        void init();

    private:
        virtual void checkAllAccess(const function<void()> &done);
        void searchAllAccess(const string &filter, const function<void(const shared_ptr<Json::Value> &)> &callback);
        virtual void fetchArtist(const string &artistId, const function<void(const shared_ptr<Json::Value> &)> &callback);
        virtual void fetchAlbum(const string &albumId, const function<void(const shared_ptr<Json::Value> &)> &callback);
        void addAllAccessIdsToLibrary(const vector<string> &songs, const function<void(const shared_ptr<Json::Value> &)> &f);
        void fetchRadioFeed(const string &radioId, const function<void(const shared_ptr<Json::Value> &)> &f) const;

        virtual void modifySongsSync(const vector<string> &ids, const std::map<string, string> &changedStrings, const std::map<string, uint64_t> &changedInts);

        
        virtual void loadRadio(const function<void(const shared_ptr<Json::Value> &)> &f) const;
        virtual void createRadio(const string &songId, const string &name, const int type, const function<void(const shared_ptr<Json::Value> &)> &f) const;

        void fetchSharedSongs(const string &playlistId, const function<void(const shared_ptr<Json::Value> &)> &f) const;
        void fetchRatedSongs(const function<void(const shared_ptr<Json::Value> &)> &f) const;

        virtual void removeRadio(const string &radioId);
        
        void fetchLibrary(const std::function<bool(const vector<Json::Value> &)> &gotDictionaries, const std::function<void()> &gotAll);
        void fetchPlaylists(const function<bool(const string &,const vector<Json::Value> &,bool)> &callback, const function<void()> &signalEnd);
        void fetchPlaylists(const vector<Json::Value> &playlistItems, const function<bool(const string &,const vector<Json::Value> &,bool)> &callback, const function<void()> &signalEnd);
        
        void addToPlaylist(const shared_ptr<IPlaylist> localPlaylist, vector<shared_ptr<ManagedObject>> &managedObjects);
        //vector<SongEntry> addToPlaylist(const Json::Value &songs, const shared_ptr<IPlaylist> destination);
        vector<Json::Value> conceive(const Json::Value &json, GooglePlayConceiver::Hint hint, const std::string &playlistId);
        

        virtual void addSongsToPlaylist(const string &playlistId, const vector<string> &songIds, const function<void(const shared_ptr<Json::Value> &)> &callback);
        virtual void createPlaylist(const vector<string> &songIds, const string &name, const function<void(const shared_ptr<Json::Value> &)> &callback);
        virtual void removePlaylist(const string &playlistId);
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<string> &songIds, const vector<string> &entryIds, const string &afterId, const string &before);
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId);
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId);
        virtual string userName() const;

        virtual shared_ptr<IPlaybackData> playbackDataSync(const string &uniqueId) const override final;
        
        THREAD_NS::mutex _cacheFetchMutex;
        THREAD_NS::condition_variable _cacheFetchCondition;
        bool _cacheFetched;
        
        shared_ptr<GoogleMusicProtocol> _protocol;

        friend class GooglePlaySession;
    };
}

#endif /* defined(__G_Ear__GooglePlaySessionImpl__) */
