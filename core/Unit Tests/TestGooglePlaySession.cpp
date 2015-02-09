//
//  TestGooglePlaySession.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#include <map>
#include "tut/tut.hpp"
#include "GooglePlaySession.h"
#include "json.h"
#include "GooglePlayDirectSong.h"
#include "GooglePlayRadio.h"
#include "IConceiver.h"


using namespace Gear;

namespace tut
{
    struct googleplaysessioninfo
    {
    };
    
    typedef test_group<googleplaysessioninfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("GooglePlaySession");
    
    class MockConceiver final : public IConceiver
    {
    public:
        MockConceiver(const Json::Value &value, const weak_ptr<GooglePlaySession> &session, int hint)
            : IConceiver(session),
            _session(session)
        {
        }
        
        SongEntry create(const string &unique) const
        {
            auto session = _session.lock();
            if (session) {
                auto ret = shared_ptr<GooglePlayDirectSong>(new GooglePlayDirectSong(unique, session));
                ret->setStringForKey("title",u(unique));
                return SongEntry(ret);
            }
            return SongEntry();
        }
        
        virtual const vector<SongEntry> songs() const
        {
            vector<SongEntry> ret;
            ret.push_back(create("hihi"));
            ret.push_back(create("haha"));
            ret.push_back(create("muhu"));
            return ret;
        }
        
        virtual const vector<shared_ptr<IArtist>> artists() const
        {
            return vector<shared_ptr<IArtist>>();
        }
        
        virtual const vector<shared_ptr<IAlbum>> albums() const
        {
            return vector<shared_ptr<IAlbum>>();
        }
        
        virtual const vector<shared_ptr<IPlaylist>> playlists() const
        {
            vector<shared_ptr<IPlaylist>> ret;
            const weak_ptr<GooglePlaySession> session;
            ret.push_back(shared_ptr<IPlaylist>(new GooglePlayRadio(u("Radio 1"), "idididid", session)));
            return ret;
        }
        
        virtual const shared_ptr<StoredPlaylist> playlistWithSongs(const vector<shared_ptr<ISong>> &songs) const
        {
            return nullptr;
        }
        
        virtual const vector<string> ids() const
        {
            return vector<string>();
        }
        
    private:
        weak_ptr<GooglePlaySession> _session;
    };
    
    class TestGooglePlaySession final : public GooglePlaySession
    {
    public:
        TestGooglePlaySession(const SharedFromThis<TestGooglePlaySession> &shared)
            : GooglePlaySession(shared, unique_ptr<const IConceiverFactory<GooglePlaySession>>(new ConceiverFactory<MockConceiver, GooglePlaySession>()), nullptr)
        {
        }
        
        static shared_ptr<TestGooglePlaySession> create()
        {
            return SharedFromThis<TestGooglePlaySession>().get();
        }
        
        virtual void removeRadio(const string &radioId) {}
        virtual void fetchLibrary() {}
        
        virtual void setFilterPredicate(const SongPredicate &f) {}
        
        virtual void changePlaylistNameSync(const string &newName, const string &playlistId) {}
        virtual void deletePlaylistEntriesSync(const vector<string> &entryIds, const vector<string> &songIds, const string &playlistId) {}
        virtual void removePlaylist(const string &playlistId) {}
        virtual const shared_ptr<Json::Value> createPlaylistSync(const vector<string> &strongIds, const string &name)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual void changePlaylistOrderSync(const string &playlistId, const vector<string> &songIds, const vector<string> &entryIds, const string &afterId, const string &before) {}
        
        virtual void modifySongsSync(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts) {}
        
        virtual void connect() {}
        virtual bool connectSync(const string &user, const string &keychainData)
        {
            return true;
        }
        virtual const shared_ptr<Json::Value> fetchSharedSongs(const string &radioId) const
        {
            return nullptr;
        }
        
    private:
        virtual const shared_ptr<Json::Value> searchAllAccess(const string &filter)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual shared_ptr<PlaybackData> playbackDataSync(const ISong &song) const
        {
            return shared_ptr<PlaybackData>();
        }
        
        virtual const shared_ptr<Json::Value>fetchArtist(const string &artistId)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual const shared_ptr<Json::Value>fetchAlbum(const string &albumId)
        {
            return shared_ptr<Json::Value>(new Json::Value());
        }
        
        virtual shared_ptr<Json::Value> addAllAccessIdsToLibrarySync(const vector<string> &songs)
        {
            return nullptr;
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
        
        virtual void fetchPlaylists() {}
        
        virtual const shared_ptr<Json::Value> addSongsToPlaylistSync(const string &playlistId, const vector<string> &songIds)
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
        
        virtual const shared_ptr<Json::Value> fetchRatedSongs() const
        {
            return nullptr;
        }
    };
    
    template<>
    template<>
    void testobject::test<1>()
    {
        shared_ptr<TestGooglePlaySession> pSession = TestGooglePlaySession::create();
        
        string token;
        auto songs = pSession->searchSync(u("Skinny"), token);
        ensure("count", songs.size() == 3);
        ensure("1", songs[0].song()->title() == u("hihi"));
        ensure("2", songs[1].song()->uniqueId() == "haha");
        
        // re-use same token must return 0
        songs = pSession->searchSync(u("Skinny"), token);
        ensure("count0", songs.empty());
        
        // then again Skinny
        string token2;
        songs = pSession->searchSync(u("Skinny"), token2);
        ensure("count", songs.size() == 3);
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        // radio
        shared_ptr<TestGooglePlaySession> pSession = TestGooglePlaySession::create();

        auto playlists = pSession->categories();
        auto arrIt = find_if(playlists.begin(), playlists.end(), [](const PlaylistCategory &category){
            return category.title() == u("Radio");
        });
        
        ensure("radiopresent", arrIt != playlists.end());
        
        auto radioPlaylists = arrIt->playlists();
        ensure_equals("zerobeforefetch", radioPlaylists.size(), 0);
        
        pSession->initialFetchSync();
        
        playlists = pSession->categories();
        arrIt = find_if(playlists.begin(), playlists.end(), [](const PlaylistCategory &category){
            return category.title() == u("Radio");
        });
        radioPlaylists = arrIt->playlists();
        ensure_equals("afterfetch", radioPlaylists.size(), 1);
        
        auto firstPlaylist = arrIt->playlists().at(0);
        ensure("name", firstPlaylist->name() == u("Radio 1"));
    }
}