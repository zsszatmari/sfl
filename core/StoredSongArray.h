//
//  StoredSongArray.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/10/13.
//
//

#ifndef __G_Ear_Player__StoredSongArray__
#define __G_Ear_Player__StoredSongArray__

#include "SessionSongArray.h"

namespace Gear
{
    class ISongIntent;
    
    class StoredSongArray final : public SessionSongArray
    {
    public:
        StoredSongArray(const shared_ptr<ISession> &session, const shared_ptr<IPlaylist> playlist);
        void addSongs(const vector<SongEntry> &songs, bool append = true);
        
    private:
        virtual const bool orderedArray() const;
        
        const weak_ptr<IPlaylist> _playlist;
    };
}

#endif /* defined(__G_Ear_Player__StoredSongArray__) */
