//
//  ISongNoIndexIntent.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/17/13.
//
//

#include <algorithm>
#include "ISongNoIndexIntent.h"
#include "SongEntry.h"
#include "ISong.h"

namespace Gear {
    
#define method ISongNoIndexIntent::
    
    void method apply(const vector<SongEntry> &entries) const
    {
        vector<shared_ptr<ISong>> songs;
        songs.reserve(entries.size());
        transform(entries.begin(), entries.end(), back_inserter(songs), [](const SongEntry &rhs){
           
            return rhs.song();
        });
        
        apply(songs);
    }
}