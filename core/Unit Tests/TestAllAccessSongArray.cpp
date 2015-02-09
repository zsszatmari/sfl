//
//  TestAllAccessSongArray.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#include "tut/tut.hpp"
#include "AllAccessSongArray.h"
#include "ISession.h"
#include "GooglePlayDirectSong.h"
#include "InstantExecutor.h"
#include "SongManipulationSession.h"

using namespace Gear;
using std::make_pair;

namespace tut
{
    struct allaccesssongarrayinfo
    {
    };
    
    typedef test_group<allaccesssongarrayinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("AllAccessSongArray");
    
    
    class MockService final : public SongManipulationSession
    {
    public:
        MockService() :
            SongManipulationSession(shared_ptr<IApp>()),
            _searchCount(0)
        {
        }
        
        virtual const vector<PlaylistCategory> categories() const
        {
            return vector<PlaylistCategory>();
        }
        
        static SongEntry create(const string &unique)
        {
            shared_ptr<MockService> dummy(new MockService());
            auto ret = shared_ptr<GooglePlayDirectSong>(new GooglePlayDirectSong(unique, dummy));
            
            ret->setStringForKey("title",u(unique));
            return SongEntry(ret);
        }
        
        void add(const string &unique)
        {
            _songs.push_back(create(unique));
        }
        
        virtual void refresh() {}
        
        virtual const vector<SongEntry> searchSync(const string &filter, string &token)
        {
            ++_searchCount;
            if (token != "aaa") {
                token = "aaa";
                return _songs;
            } else {
                return vector<SongEntry>();
            }
        }
        
        virtual shared_ptr<PlaybackData> playbackDataSync(const ISong &song) const
        {
            return shared_ptr<PlaybackData>();
        }
        
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
        {
        }
        
        vector<SongEntry> _songs;
        int _searchCount;
        
        virtual shared_ptr<IPlaylist> libraryPlaylist() { return nullptr; }
        virtual void connect() {}
        
    private:
        
        virtual vector<shared_ptr<IPlaylist>> playlists()
        {
            return vector<shared_ptr<IPlaylist>>();
        }
        
        virtual void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name)
        {
        }
        
        virtual void modifySongs(const vector<string> &ids, const map<string, string> &changedStrings, const map<string, uint64_t> &changedInts)
        {
        }        
    };
    
    template<>
    template<>
    void testobject::test<1>()
    {
        shared_ptr<MockService> service(new MockService());
        service->add("1");
        service->add("2");
        service->add("3");
        
        auto allAccess = AllAccessSongArray::create(service);
        ensure("zero", allAccess->size() == 0);
        
        shared_ptr<InstantExecutor> instant(new InstantExecutor());
        bool change = false;
        const function<void()> f = [&]{
            change = true;
        };
        auto conn = allAccess->updatedEvent().connectWithExecutor(f, instant);
        
        allAccess->setFilterPredicate(SongPredicate("", u("Skinny"), SongPredicate::Contains()));
        milliseconds w(10);
        sleep_for(w);
        
        
        ensure("changed", change);
        ensure("mocksearch", service->_searchCount == 2);
        ensure_equals("count", allAccess->size(), 3);
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        shared_ptr<MockService> service(new MockService());
        service->add("1");
        service->add("2");
        service->add("3");

        auto allAccess = AllAccessSongArray::create(service);
        ensure("notcontains", !allAccess->contains(service->_songs[2]));

        allAccess->setFilterPredicate(SongPredicate("", u("Skinny"), SongPredicate::Contains()));

        milliseconds w(10);
        sleep_for(w);

        ensure("contains", allAccess->contains(service->_songs[2]));
        
        ensure("at", allAccess->at(1) == service->_songs[1]);
        ensure("indexof", allAccess->indexOf(service->_songs[1]) == 1);
        auto testSong = MockService::create("iszkiri");
        ensure("indexofnot", allAccess->indexOf(testSong) == ((unsigned long)-1));
    }
               
    template<>
    template<>
    void testobject::test<3>()
    {        
        shared_ptr<MockService> service(new MockService());
        service->add("2");
        service->add("3");
        service->add("1");
        
        auto allAccess = AllAccessSongArray::create(service);
        ensure("zero", allAccess->size() == 0);
        ensure("notcontains", !allAccess->contains(service->_songs[2]));
        
        allAccess->setFilterPredicate(SongPredicate("", u("Skinny"), SongPredicate::Contains()));
        milliseconds w(10);
        sleep_for(w);
        
        {
            pair<string,bool> p[] = {make_pair("title", true)};
            allAccess->setSortDescriptor(SortDescriptor(init<vector<pair<string,bool>>>(p)));
            
            ensure_equals("sortnumber", allAccess->size(), 3);
            
            ensure("sorta0", allAccess->at(0).song()->title() == u("1"));
            ensure("sorta1", allAccess->at(1).song()->title() == u("2"));
            ensure("sorta2", allAccess->at(2).song()->title() == u("3"));
        }
        
        {
            pair<string,bool> p[] = {make_pair("title", false)};
            allAccess->setSortDescriptor(SortDescriptor(init<vector<pair<string,bool>>>(p)));
            
            ensure("sortd0", allAccess->at(0).song()->title() == u("3"));
            ensure("sortd1", allAccess->at(1).song()->title() == u("2"));
            ensure("sortd2", allAccess->at(2).song()->title() == u("1"));
        }
        
        ensure_equals("count", allAccess->size(), 3);
    }
}