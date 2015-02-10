//
//  WeakSongEntry.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/6/13.
//
//

#ifndef __G_Ear_Player__WeakSongEntry__
#define __G_Ear_Player__WeakSongEntry__

#include <string>
#include "stdplus.h"
#include WEAK_H
using MEMORY_NS::weak_ptr;

namespace Gear
{
    using std::string;
    
    class SongEntry;
    class ISong;
    
    class core_export WeakSongEntry final
    {
    public:
        WeakSongEntry();
        WeakSongEntry(const SongEntry &entry);
        SongEntry lock() const;
        
    private:
        weak_ptr<ISong> _song;
        string _entryId;
    };
}

#endif /* defined(__G_Ear_Player__WeakSongEntry__) */
