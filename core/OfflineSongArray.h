//
//  OfflineSongArray.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 01/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__OfflineSongArray__
#define __G_Ear_core__OfflineSongArray__

#include "SortedSongArray.h"
#include SHAREDFROMTHIS_H
#include ATOMIC_H

namespace Gear
{
    class OfflineSongArray : public SortedSongArray, public MEMORY_NS::enable_shared_from_this<OfflineSongArray>
    {
    public:
        static shared_ptr<OfflineSongArray> create(const shared_ptr<ISession> &);
        
        void refresh();
        virtual vector<shared_ptr<ISongIntent>> songIntents(const std::vector<SongEntry> &songs, const std::string &fieldHint) const;
        
    private:
        OfflineSongArray(const shared_ptr<ISession> &);
        weak_ptr<ISession> _session;
        Base::SerialExecutor _executor;
        ATOMIC_NS::atomic_int _refreshCount;
    };
}

#endif /* defined(__G_Ear_core__OfflineSongArray__) */
