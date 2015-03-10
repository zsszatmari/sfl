//
//  SortedSongArray.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/4/13.
//
//

#ifndef __G_Ear__SortedSongArray__
#define __G_Ear__SortedSongArray__

#include <set>
#include "stdplus.h"
#include "ISongArray.h"
#include "SortDescriptor.h"
#include "SongPredicate.h"
#include "AtomicPtr.h"
#include "ConstPtr.h"
#include "SerialExecutor.h"
#include "Chain.h"

namespace Gear
{
    using std::set;
    
    class SortedSongArray : public ISongArray
    {
    public:
        SortedSongArray();
        //SortedSongArray(const shared_ptr<SortedSongArray> &commonUnsorted);
        virtual unsigned long size() const;
        virtual bool contains(const SongEntry &song) const;
        virtual const SongEntry at(unsigned long pos) const;
        virtual unsigned long indexOf(const SongEntry &rhs) const;
        
        virtual ValidPtr<const Chain<SongEntry>> songs() const;
        
        //virtual void setFilter(const string &aFilter, const string &category);
        virtual void setFilterPredicate(const SongPredicate &f);
        virtual void setSecondaryFilterPredicate(const SongPredicate &f);
        virtual SongPredicate secondaryFilterPredicate() const;
        virtual void setSortDescriptor(const SortDescriptor &rhs);
        virtual SortDescriptor sortDescriptor() const;
        virtual void resetCache();
        // set or append potentially from other thread
        void setAllSongs(const vector<SongEntry> &songs, bool append = false, bool overwrite = false);
        void removeSongs(const vector<SongEntry> &songs);
        void removeSongsOtherThan(const vector<SongEntry> &songs, bool forceRefilter = false);
        bool moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before);
        
        // unsorted, unfiltered values
        virtual ValidPtr<const vector<SongEntry>> rawSongs() const;
        // possibly unsorted, but filtered values
        virtual ValidPtr<const vector<SongEntry>> filteredSongs() const;
        
        Base::EventConnector updatedEvent();
        void alertListener();
        

    protected:
        const SongPredicate filterPredicate() const;
        void appendAllSongs(vector<SongEntry> &songs);
        const shared_ptr<ISong> contained(ISong *song);
        
        void refilter();
        void setDataReplaced();
    
        
        struct Unsorted final
        {
        public:
            Unsorted();
        
            mutable mutex _allSongsMutex;
            
            vector<SongEntry> _allSongs;
            // need to be ordered for removeSongsOtherThan()
            set<SongEntry> _allSongsSet;
        };
        ConstPtr<Unsorted> _unsorted;
        
        virtual shared_ptr<const vector<SongEntry>> all() const;
        void setRawSongs(const ValidPtr<const vector<SongEntry>> &);
        
        void alertUnsortedChange();
        
    private:
        struct Conditions final
        {
        public:
            Conditions();
            
            SongPredicate filterPredicate;
            SongPredicate secondaryFilterPredicate;
            SortDescriptor sortDescriptor;
        };
        
        struct State final
        {
        public:
            State();
            
            AtomicPtr<vector<SongEntry>> unsortedSongs;
            AtomicPtr<vector<SongEntry>> filteredSongs;
            AtomicPtr<Chain<SongEntry>> sortedFilteredSongs;
            AtomicPtr<Conditions> conditions;
            AtomicPtr<bool> filtering;
            
            // must be invoked in the background
            void doSortFiltered(const weak_ptr<const bool> &weakFutur, bool relevanceBased);
            
            Base::SerialExecutor _worker;
            
            Base::EventSignal _updatedSignal;
            void doAlertListener();
            
            AtomicPtr<vector<function<void(Chain<SongEntry> &)>>> pending;
            atomic<bool> _dataReplaced;
        };
        
        ConstPtr<State> _state;
        
        void apply(const decltype(State::pending)::element_type::value_type &minor);
        
        //shared_ptr<bool> _sorting;
        
        virtual bool relevanceBased() const;
        
        bool allSongsSorted() const;
        
        void filterRaw();
        virtual const bool orderedArray() const;
        
        shared_ptr<const vector<SongEntry>> filteredSongs(const unsigned long atleast, const bool keepSize = false) const;
        virtual bool filterNeeded() const;
    };
}

#endif /* defined(__G_Ear__SortedSongArray__) */
