//
//  UnionSongArray.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/27/13.
//
//

#ifndef __G_Ear_Player__UnionSongArray__
#define __G_Ear_Player__UnionSongArray__

#include "SortedSongArray.h"
#include "SerialExecutor.h"
#include ATOMIC_H
#include SHAREDFROMTHIS_H

namespace Gear
{
    class UnionSongArray final : public SortedSongArray, public MEMORY_NS::enable_shared_from_this<UnionSongArray>
    {
    public:
        static shared_ptr<UnionSongArray> create();
        void setSongArrays(const vector<shared_ptr<SortedSongArray>> &songArrays);
        virtual void setFilterPredicate(const SongPredicate &f);
        virtual const string emptyText() const;
        virtual bool needsPredicate() const;
        
    private:
        UnionSongArray();
        
        Base::SerialExecutor _resortExecutor;
        ATOMIC_NS::atomic_int _resortCount;

        mutable THREAD_NS::recursive_mutex _songArraysMutex;
        vector<shared_ptr<SortedSongArray>> _songArrays;
        vector<shared_ptr<SortedSongArray>> songArrays() const;

        void resort();
        vector<Base::SignalConnection> _updateConnections;
        AtomicPtr<std::map<shared_ptr<ISongArray>,std::vector<std::string>>> _identifiers;
        
        shared_ptr<ISongArray> arrayForSong(const SongEntry &song) const;
        
        virtual void songsAccessed() const;
        virtual const bool orderedArray() const;
        virtual bool relevanceBased() const;
    };
}

#endif /* defined(__G_Ear_Player__UnionSongArray__) */
