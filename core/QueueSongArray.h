//
//  QueueSongArray.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 28/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__QueueSongArray__
#define __G_Ear_core__QueueSongArray__

#include "SortedSongArray.h"
#include "AtomicPtr.h"
#include "AtomicMutablePtr.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    class IPlayer;

    class QueueSongArray : public SortedSongArray, public MEMORY_NS::enable_shared_from_this<QueueSongArray>
    {
    public:
        static shared_ptr<QueueSongArray> instance();
        static shared_ptr<QueueSongArray> create(const shared_ptr<IPlayer> &player);

        SongEntry whatToPlayNextAfter(const SongEntry &songPlayed, bool forward = true);
        SongEntry nextInOrder(const SongEntry &songPlayed, bool forward = true);
        void setArrayForJumping(const shared_ptr<ISongArray> &array);
        void recalculate();
        void recalculate(const shared_ptr<ISongArray> &array, const SongEntry &entry);
        virtual const bool orderedArray() const;
        void enqueue(const vector<SongEntry> &entries, bool asNext);
        void removeFromQueue(const vector<SongEntry> &entries);
        bool moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before);
        virtual void setNoRepeatBase(const SongEntry &song);
        
        void willRemoveSongs(const vector<SongEntry> &songs);

    private:
        QueueSongArray(const shared_ptr<IPlayer> player);

        void init();
        void recalculate(const SongEntry &centered);
        shared_ptr<vector<SongEntry>> postprocess(const shared_ptr<std::vector<SongEntry>> &v);

        const shared_ptr<IPlayer> _player;
        
        AtomicPtr<vector<SongEntry>> _all;
        AtomicPtr<vector<SongEntry>> _outOfOrder;
        AtomicMutablePtr<ISongArray> _arrayForJumping;
        Base::SerialExecutor _queueCalculator;

        Base::SignalConnection _repeatConnection;
        Base::SignalConnection _shuffleConnection;

        SongView _updateConnection;
    };
}

#endif /* defined(__G_Ear_core__QueueSongArray__) */
