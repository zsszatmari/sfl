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
#include "Predicate.h"

namespace Gear
{
    using std::set;
    
    class SortedSongArray : public ISongArray
    {
    public:
        SortedSongArray(const shared_ptr<IDb> &db, const ClientDb::Predicate &inherentPredicate, const std::map<string,string> &traits);

        //virtual ValidPtr<const Chain<SongEntry>> songs() const;
        virtual SongView songs(const std::function<void(const SongView &, SongView::Event,size_t,size_t)> &callback) const final;

        virtual bool contains(const SongEntry &song) const;
        
        virtual void resetCache();
        virtual void setFilterPredicate(const SongPredicate &f);
        virtual void setSecondaryFilterPredicate(const SongPredicate &f);

        virtual void removeSongs(const vector<SongEntry> &songs);
        //virtual void removeSongsOtherThan(const vector<SongEntry> &songs, bool forceRefilter = false);
        virtual bool moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before);

        virtual void setSortDescriptor(const SortDescriptor &rhs);
        // that is, we copy these songs into this playlist
        void setAllSongs(const vector<SongEntry> &songs, bool append = false, bool overwrite = false);
        virtual unsigned long indexOf(const SongEntry &rhs) const override final;
        virtual uint32_t sizeSync() const override final;

        virtual void setNoRepeatBase(const SongEntry &song) override;
        
        virtual Base::EventConnector updatedEvent();
        
        ClientDb::Predicate inherentPredicate() const;

        virtual SongEntry nextShuffledSongRelativeTo(const SongEntry &song, bool ascending, const bool repeat) const override final;
        virtual shared_ptr<InspectionContext> inspectionContext() const;
        virtual shared_ptr<const vector<SongEntry>> songsReallySlow() const override final;

        virtual bool empty() const override final;
        SongPredicate filterPredicate() const;
        void setInherentPredicate(const ClientDb::Predicate &predicate);
        
        shared_ptr<IDb> db() const;
        virtual SortDescriptor sortDescriptor() const;
        virtual string predicateAsString() const override;
        virtual void songsLeft(unsigned long count) override final;

    protected:
        void appendAllSongs(const vector<SongEntry> &songs);
        
    private:    
        class InspectionContextImpl : public InspectionContext
        {
        public:
            InspectionContextImpl();

            int size;
            std::vector<std::pair<uint32_t,uint32_t>> shuffleHashToRowId;
            std::vector<std::string> allEntryIds;
        };
        mutable weak_ptr<InspectionContextImpl> _inspectionContext;
        shared_ptr<InspectionContextImpl> inspectionContextImpl() const;

        virtual vector<std::string> all(const string &field) const;
        virtual std::map<int,std::string> mapping(const string &field, int slots) const override final;
        virtual SongEntry atSync(size_t index) const override final;

        virtual void songsAccessed() const;

        shared_ptr<IDb> _db;

        uint32_t shuffleHashForUniqueId(const string &uniqueId) const;

        ClientDb::SortDescriptor orderBy() const;
        ClientDb::Predicate predicate() const;
        
        uint32_t shuffleSeed;
        uint32_t firstShuffleNoRepeatSaltedHash;

        const std::map<string,string> _traits;

        SongPredicate _filterPredicate;
        Base::EventSignal _eventSignal;
        SortDescriptor _sortDescriptor;
        ClientDb::Predicate _searchPredicate;
        ClientDb::Predicate _secondaryPredicate;
        ClientDb::Predicate _inherentPredicate;
    };
}

#endif /* defined(__G_Ear__SortedSongArray__) */
