//
//  MockSession.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/14/13.
//
//

#ifndef __G_Ear_Player__MockSession__
#define __G_Ear_Player__MockSession__

#include "GooglePlaySession.h"
#include "GooglePlayConceiver.h"
#include "GooglePlayDirectSong.h"
#include "Document.h"

namespace tut
{

    using namespace Gear;
    
    class MockSession final : public GooglePlaySession
    {
    public:
        MockSession() :
        GooglePlaySession(SharedFromThis<MockSession>(),
                          unique_ptr<const IConceiverFactory<GooglePlaySession>>(new ConceiverFactory<GooglePlayConceiver, GooglePlaySession>()), nullptr) // dummy
        {
            _document->open("teststore.db");
        }
        
        MockSession(const SharedFromThis<MockSession> &abuse) :
        GooglePlaySession(abuse, unique_ptr<const IConceiverFactory<GooglePlaySession>>(new ConceiverFactory<GooglePlayConceiver, GooglePlaySession>()), nullptr)
        {
        }
        
        virtual const vector<SongEntry> searchSync(const string &filter, string &token) {return vector<SongEntry>();}
        
        virtual shared_ptr<PlaybackData> playbackDataSync(const ISong &song) const {
            return shared_ptr<PlaybackData>();
        }
        
        virtual shared_ptr<Json::Value> addAllAccessIdsToLibrarySync(const vector<string> &songs)
        {
            return nullptr;
        }
        
        virtual const vector<SongEntry> radioSongsSync(const string &radioId)
        {
            lastRadioId = radioId;
            vector<SongEntry> ret;
            ret.push_back(SongEntry(shared_ptr<ISong>(new GooglePlayDirectSong("hihi", weak_ptr<GooglePlaySession>()))));
            
            return ret;
        }
        
        virtual void removeRadio(const string &radioId) {}
        virtual void fetchLibrary() {}
        
        virtual void setFilterPredicate(const SongPredicate &f) {}
        
        virtual void fetchPlaylists() {}
        
        mutable string lastRadioId;
        
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId) {}
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId) {}
        virtual void removePlaylist(const string &playlistId) {}
        virtual const shared_ptr<Json::Value> createPlaylistSync(const vector<string> &strongIds, const string &name)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<string> &songIds, const vector<string> &entryIds, const string &afterId, const string &before) {}
        
        virtual void modifySongsSync(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts) {}
        
        virtual shared_ptr<IPlaylist> libraryPlaylist() { return nullptr; }
        virtual void connect() {}
        virtual const shared_ptr<Json::Value> fetchSharedSongs(const string &radioId) const
        {
            return nullptr;
        }
        virtual bool connectSync(const string &user, const string &keychainData){
            return true;
        }
        virtual const shared_ptr<Json::Value> fetchRatedSongs() const
        {
            return nullptr;
        }
        
    private:
        virtual const shared_ptr<Json::Value> addSongsToPlaylistSync(const string &playlistId, const vector<string> &songIds)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual const shared_ptr<Json::Value> searchAllAccess(const string &filter)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual const shared_ptr<Json::Value> fetchArtist(const string &artistId)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual const shared_ptr<Json::Value> fetchAlbum(const string &albumId)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual void addAllAccessIdsToLibrary(const vector<string> &songs)
        {
        }
        
        virtual const shared_ptr<Json::Value> fetchRadioFeed(const string &radioId) const
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual const shared_ptr<Json::Value> loadRadio() const
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual const shared_ptr<Json::Value> createRadio(const string &songId, const string &name, const int type) const
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual const shared_ptr<Json::Value> createRadio(const string &songId, const string &name) const
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual const shared_ptr<Json::Value> createPlaylistSync(const vector<string> &strongIds)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual string userName() const
        {
            return "testuser";
        }
    };
}

#endif /* defined(__G_Ear_Player__MockSession__) */
