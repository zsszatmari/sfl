//
//  ISongArray.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/24/13.
//
//

#ifndef G_Ear_ISongArray_h
#define G_Ear_ISongArray_h

#include <vector>
#include "stdplus.h"
#include "SongEntry.h"
#include "EventSignal.h"
#include "SongView.h"

namespace Gear
{
    using std::vector;
    using std::pair;
    
    class ISongIntent;
    class SortDescriptor;
    class SongPredicate;
    template<class T>
    class Chain;
    template<class T>
    class ValidPtr;
    
    class ISongArray
    {
    public:
        ISongArray();
        virtual ~ISongArray();
        virtual bool operator==(const ISongArray &rhs);
        
        //virtual ValidPtr<const Chain<SongEntry>> songs() const = 0;
        // intended to be used from the main queue
        virtual SongView songs(const std::function<void(const SongView &,SongView::Event,size_t,size_t)> &callback) const = 0;
        

        virtual bool contains(const SongEntry &song) const = 0;
        
        static const size_t NotFound;
        virtual unsigned long indexOf(const SongEntry &rhs) const = 0;
        virtual void resetCache() = 0;
        virtual void setFilterPredicate(const SongPredicate &f) = 0;
        virtual void setSecondaryFilterPredicate(const SongPredicate &f);
        virtual SongPredicate secondaryFilterPredicate() const;
        virtual vector<string> forbiddenColumns() const;
        virtual vector<shared_ptr<ISongIntent>> songIntents(const vector<SongEntry> &songs, const string &fieldHint) const;
        virtual vector<pair<string, vector<shared_ptr<ISongIntent>>>> songIntentSubMenus(const vector<SongEntry> &songs) const;
        virtual const string emptyText() const;
        virtual void songsLeft(unsigned long count);
        // unsorted, unordered values (makes no sense anymore)
        //virtual ValidPtr<const vector<SongEntry>> rawSongs() const = 0;
        // possibly unsorted, but filtered values
        //virtual ValidPtr<const vector<SongEntry>> filteredSongs() const = 0;
        
        // might return nullptr
        virtual SongEntry nextShuffledSongRelativeTo(const SongEntry &song, bool ascending, const bool repeat) const = 0;
        virtual void setNoRepeatBase(const SongEntry &song) = 0;
        virtual const bool orderedArray() const;

        virtual void removeSongs(const vector<SongEntry> &songs) = 0;
        // this is dangerous: lot of memory needed
        //virtual void removeSongsOtherThan(const vector<SongEntry> &songs, bool forceRefilter = false) = 0;
        virtual bool moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before) = 0;
        virtual void setSortDescriptor(const SortDescriptor &rhs) = 0;

        
        virtual Base::EventConnector updatedEvent() = 0;
        void freeUpMemory();
        
        virtual bool needsPredicate() const;

        class InspectionContext
        {
        public:
            virtual ~InspectionContext() {}
        };
        virtual shared_ptr<InspectionContext> inspectionContext() const = 0;
        

        //this is a deprecated, synchronous interface. still useful for testing, but avoid this otherwise...
        virtual shared_ptr<const vector<SongEntry>> songsReallySlow() const = 0; 
        virtual bool empty() const = 0;
        virtual std::map<int,std::string> mapping(const string &field, int slots) const = 0;
        virtual SortDescriptor sortDescriptor() const = 0;
        virtual string predicateAsString() const = 0;

    private:
        ISongArray(const ISongArray &rhs); // delete
        ISongArray &operator=(const ISongArray &rhs); // delete
    
        virtual vector<std::string> all(const string &field) const = 0;
        virtual SongEntry atSync(size_t index) const = 0;

        virtual uint32_t sizeSync() const = 0;

        friend class IPlaylist;
        friend class UnionSongArray;
        friend class QueueSongArray;
    };
}

#endif
