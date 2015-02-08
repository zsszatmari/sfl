//
//  DragManager.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/31/13.
//
//

#ifndef __G_Ear__DragManager__
#define __G_Ear__DragManager__

#include <vector>
#include "stdplus.h"
#include "ISong.h"
#include "ISongArray.h"

namespace Gear
{
    class DragManager final
    {
    public:
        DragManager();
        void dragSongs(const weak_ptr<ISongArray>, const vector<SongEntry> &songs);
        const vector<SongEntry> &draggedEntries() const;
        const weak_ptr<ISongArray> &songsSource() const;
    
    private:
        weak_ptr<ISongArray> _songsSource;
        vector<SongEntry> _draggedEntries;
        
        DragManager(const DragManager &rhs); // delete
        const DragManager &operator=(const DragManager &rhs); // delete
    };
}

#endif /* defined(__G_Ear__DragManager__) */
