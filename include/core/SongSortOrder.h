//
//  SongSortOrder.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/15/13.
//
//

#ifndef __G_Ear_Player__SongSortOrder__
#define __G_Ear_Player__SongSortOrder__

#include "stdplus.h"
#include "SortDescriptor.h"
#include <functional>

namespace Gear
{
    class SongEntry;
    using std::function;
    
    class core_export SongSortOrder
    {
    public:
        static vector<string> defaultSortOrder();
        static SortDescriptor sortDescriptor(const string &key = "", bool ascending = true);
        
        static function<bool(const SongEntry &,const SongEntry &)> comparator(const SortDescriptor &sortDescriptor);
    };
}

#endif /* defined(__G_Ear_Player__SongSortOrder__) */
