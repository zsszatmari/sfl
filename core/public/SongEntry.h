//
//  ISongEntry.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/20/13.
//
//

#ifndef __G_Ear_Player__ISongEntry__
#define __G_Ear_Player__ISongEntry__

#include "ISong.h"
#include MEMORY_H
#include "json-forwards.h"

namespace Gear
{
    class SongEntrySong;
    class SongData;

    class core_export SongEntry final
    {
    public:
        SongEntry();
        //SongEntry(const SongEntry &rhs);
        SongEntry(const SongData &value);
        SongEntry clone(int16_t replicaDiff) const;
        SongEntry cloneWith(int16_t replica) const;
        SongEntry unclone() const;
        
        int16_t replica() const;
        const shared_ptr<ISong> song() const;
        string entryId() const;
        
        bool operator==(const SongEntry &rhs) const;
        bool operator!=(const SongEntry &rhs) const;
        bool operator<(const SongEntry &rhs) const;
        explicit operator bool() const;

        SongEntry(const string &source, const string &playlist, const string &entryId, const string &songId);
        Json::Value toJson() const;
        
    private:
        SongEntry(const shared_ptr<ISong> &song, const string &entryId);
        
        shared_ptr<SongEntrySong> _song;
        
        friend void swap(SongEntry& a, SongEntry& b);

        friend class Db;
        friend class GooglePlaySession;
        friend class WeakSongEntry;
        friend class YouTubeSession;
    };
    
    std::size_t hash_value(const SongEntry &rhs);
}

#endif /* defined(__G_Ear_Player__ISongEntry__) */
