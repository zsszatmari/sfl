//
//  TestISongArray.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/26/13.
//
//

#include "tut/tut.hpp"
#include "ISongArray.h"


using namespace Gear;

namespace tut
{
    struct isongarrayinfo
    {
    };
    
    typedef test_group<isongarrayinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("ISongArray");
    
    class TestSong final : public ISong
    {
    public:
        virtual const string uniqueId() const {return _uniqueId;}
        virtual const string stringForKey(const string &key) const {return "";}
        virtual const uint64_t uIntForKey(const string &key) const {return 0;}
        virtual const float floatForKey(const string &key) const {return 0.0f;}
        virtual const string string16ForKey(const string &key) const {return string();}
        
        virtual void setPlayCount(const uint32_t &value) {}
        virtual void setLastPlayed(const uint64_t &value) {}
        virtual void setRating(int rating) {}
        
        virtual bool playlist() const {return false;}
        virtual void removeFromPlaylist() {}
        
        virtual void save() {}
        virtual bool valid() const {return 0;}
        
        // this method will block so it's best to call it synchronously
        virtual shared_ptr<PlaybackData> playbackDataSync() {return shared_ptr<PlaybackData>();}
        
        virtual void setStringForKey(const string &key, const string &value) {}
        virtual void setUintForKey(const string &key, const uint64_t &value) {}
        virtual void setFloatForKey(const string &key, const float &value) {}
        
        string _uniqueId;
    };
    
    class TestSongArray final : public ISongArray {
        
    public:
        virtual ~TestSongArray()
        {
        }
        
        virtual const vector<SongEntry> & all() const
        {
            return _all;
        }

        
        virtual unsigned long size() const
        {
            return 0;
        }
        
        virtual bool contains(const SongEntry &song) const
        {
            return false;
        }
        
        virtual const SongEntry at(unsigned long pos) const
        {
            return _all.at(pos);
        }
        
        virtual unsigned long indexOf(const SongEntry &rhs) const
        {
            return -1;
        }
        
        virtual void resetCache()
        {
        }
        
        virtual void setSortDescriptor(const SortDescriptor &rhs)
        {
        }
        
        virtual void setFilter(const string &filter, const string &category)
        {
        }
        
        virtual void setFilterPredicate(const SongPredicate &f)
        {
        }
        
        void addUniqueId(const string unique)
        {
            shared_ptr<TestSong> add = shared_ptr<TestSong>(new TestSong());
            add->_uniqueId = unique;
            _all.push_back(SongEntry(add));
        }
        
        virtual const vector<SongEntry> rawSongs() const
        {
            return _all;
        }

        vector<SongEntry> _all;
    };
   
    template<>
    template<>
    void testobject::test<1>()
    {        
        // it's stochastic so must test multiple times!
        for (int i = 0 ; i < 50 ; i++) {
            
            TestSongArray songs;
            
            songs.addUniqueId("1");
            songs.addUniqueId("2");
            songs.addUniqueId("3");
            songs.addUniqueId("4");
            
            
            //cout << "song0: " << songs.at(1)->uniqueId() << endl;
            auto next1 = songs.nextShuffledSongRelativeTo(songs.at(1), true, true);
            //cout << "song1: " << next1->uniqueId() << endl;
            ensure("1no", next1 != songs.at(1));
            auto next2 = songs.nextShuffledSongRelativeTo(next1, true, true);
            //cout << "song2: " << next2->uniqueId() << endl;
            ensure("2no", next2 != songs.at(1));
            auto next3 = songs.nextShuffledSongRelativeTo(next2, true, true);
            ensure("3no", next3 != songs.at(1));
            auto next4 = songs.nextShuffledSongRelativeTo(next3, true, true);
            ensure("4yes", next4 == songs.at(1));
            
            // go back
            auto next5 = songs.nextShuffledSongRelativeTo(next4, false, true);
            ensure("5eq", next5 == next3);
            auto next6 = songs.nextShuffledSongRelativeTo(next5, false, true);
            ensure("6eq", next6 == next2);
        }
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        // no repeat
        for (int i = 0 ; i < 50 ; i++) {
            
            TestSongArray songs;
            
            songs.addUniqueId("1");
            songs.addUniqueId("2");
            songs.addUniqueId("3");
            songs.addUniqueId("4");
            
            songs.setNoRepeatBase(songs.at(1));
            
            //cout << "song0: " << songs.at(1)->uniqueId() << endl;
            auto next1 = songs.nextShuffledSongRelativeTo(songs.at(1), true, false);
            //cout << "song1: " << next1->uniqueId() << endl;
            ensure("1no", !(next1 == songs.at(1)));
            auto next2 = songs.nextShuffledSongRelativeTo(next1, true, false);
            //cout << "song2: " << next2->uniqueId() << endl;
            ensure("2no", !(next2 == songs.at(1)));
            auto next3 = songs.nextShuffledSongRelativeTo(next2, true, false);
            ensure("3no", !(next3 == songs.at(1)));
            auto next4 = songs.nextShuffledSongRelativeTo(next3, true, false);
            ensure("4yes", !next4);
            
            
            // and no going back
            auto back1 = songs.nextShuffledSongRelativeTo(next1, false, false);
            ensure("5back", !back1);
        }
    }

    
    template<>
    template<>
    void testobject::test<3>()
    {
          
        string oldNext3;
        bool change = false;
        
        // test the stocahistveness
        for (int i = 0 ; i < 50 ; i++) {
            
            TestSongArray songs;
            
            songs.addUniqueId("1");
            songs.addUniqueId("2");
            songs.addUniqueId("3");
            songs.addUniqueId("4");
            
            auto next1 = songs.nextShuffledSongRelativeTo(songs.at(1), true, true);
            auto next2 = songs.nextShuffledSongRelativeTo(next1, true, true);
            auto next3 = songs.nextShuffledSongRelativeTo(next2, true, true);
            if (!oldNext3.empty()) {
                if (oldNext3 != next3.song()->uniqueId()) {
                    change = true;
                }
            }
            oldNext3 = next3.song()->uniqueId();
        }
        ensure("stocha", change);
    }

    template<>
    template<>
    void testobject::test<4>()
    {
        // only one song!
        
        TestSongArray songs;
        
        songs.addUniqueId("1");
        
        songs.setNoRepeatBase(songs.at(0));
        
        auto next1 = songs.nextShuffledSongRelativeTo(songs.at(0), true, true);
        ensure("1y", next1 == songs.at(0));
        auto next2 = songs.nextShuffledSongRelativeTo(next1, true, true);
        ensure("2y", next2 == songs.at(0));
        auto next3 = songs.nextShuffledSongRelativeTo(next1, false, true);
        ensure("3y", next3 == songs.at(0));
        auto next4 = songs.nextShuffledSongRelativeTo(next1, false, false);
        ensure("4n", !next4);
        auto next5 = songs.nextShuffledSongRelativeTo(next1, true, false);
        ensure("5n", !next5);
    }
}