//
//  TestStoredSongArray.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/11/13.
//
//

#include <memory>
#include "tut/tut.hpp"
#include "StoredSongArray.h"
#include "MockSession.h"


using namespace Gear;

namespace tut
{    
    struct teststored
    {
        //       struct keepee{ int data; };
    };
    
    typedef test_group<teststored> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("StoredSongArray");
    
    class TestSongForSort final : public ISong
    {
    public:
        TestSongForSort(const string &str) : _str(str)
        {
        }
        
        virtual const string uniqueId() const
        {
            return _str;
        }
        
        virtual const string stringForKey(const string &key) const
        {
            return _str;
        }
        
        virtual const string string16ForKey(const string &key) const
        {
            return u(_str);
        }
        
        virtual const uint64_t uIntForKey(const string &key) const {return 1; }
        virtual const float floatForKey(const string &key) const { return 1; }
        virtual void setStringForKey(const string &key, const string &value) {}
        virtual void setUintForKey(const string &key, const uint64_t &value) {}
        virtual void setFloatForKey(const string &key, const float &value) {}

    
        virtual void save() {}
        virtual bool valid() const {return true; }
        
        virtual bool playlist() const { return false;}
        virtual void removeFromPlaylist() {}
        
        // this method will block so it's best to call it synchronously
        virtual shared_ptr<PlaybackData> playbackDataSync()
        {
            return shared_ptr<PlaybackData>();
        }
        
        
        string _str;
    };

    
    template<>
    template<>
    void testobject::test<1>()
    {
        shared_ptr<GooglePlaySession> session = shared_ptr<GooglePlaySession>(new MockSession());
        StoredSongArray array(session, nullptr);
        
        SongEntry song1 = SongEntry(shared_ptr<ISong>(new TestSongForSort("hihi")));
        SongEntry song2 = SongEntry(shared_ptr<ISong>(new TestSongForSort("epfefe")));
        SongEntry song3 = SongEntry(shared_ptr<ISong>(new TestSongForSort("ahahaha")));
        SongEntry s[] = {song1, song2, song3};
        array.addSongs(init<vector<SongEntry>>(s));
        
        ensure_equals("sizeafteradd", array.size(), 3);
        
        pair<string,bool> p("title", true);
        pair<string,bool> pa[] = {p};
        SortDescriptor sortDesc(init<vector<pair<string,bool>>>(pa));
        array.setSortDescriptor(sortDesc);
        
        ensure_equals("size", array.size(), 3);
        string uni0 = array.at(0).song()->uniqueId();
        ensure("order", uni0 == "ahahaha");
        
        //ensure("unfinished", 0);
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        shared_ptr<GooglePlaySession> session = shared_ptr<GooglePlaySession>(new MockSession());
        StoredSongArray array(session, nullptr);
        
        SongEntry song1 = SongEntry(shared_ptr<ISong>(new TestSongForSort("hihi")));
        SongEntry song2 = SongEntry(shared_ptr<ISong>(new TestSongForSort("epfefe")));
        SongEntry song3 = SongEntry(shared_ptr<ISong>(new TestSongForSort("ahahaha")));
        SongEntry song4 = SongEntry(shared_ptr<ISong>(new TestSongForSort("qqqqq")));
        SongEntry song5 = SongEntry(shared_ptr<ISong>(new TestSongForSort("ihaj")));
        SongEntry s[] = {song1, song2, song3, song4, song5};
        array.addSongs(init<vector<SongEntry>>(s));

        SongEntry m[] = {song3,song4};
        
        ensure_equals("sizebefore", array.size(), 5);
        array.moveSongs(init<vector<SongEntry>>(m), song1, song2);
        ensure_equals("sizeafter", array.size(), 5);
        
        const string s1 = array.at(0).entryId();
        const string s2 = array.at(1).entryId();
        const string s3 = array.at(2).entryId();
        const string s4 = array.at(3).entryId();
        const string s5 = array.at(4).entryId();
        
        
        ensure("1", s1 == "hihi");
        ensure("2", s2 == "ahahaha");
        ensure("3", s3 == "qqqqq");
        ensure("4", s4 == "epfefe");
        ensure("5", s5 == "ihaj");
    }
}
