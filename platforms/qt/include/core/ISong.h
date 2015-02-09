//
//  ISong.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/24/13.
//
//

#ifndef G_Ear_ISong_h
#define G_Ear_ISong_h

#include <string>
#include "stdplus.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    using std::string;
    
    class IPlaybackData;
    class ISession;
    class OfflineState;
    
    class core_export ISong
    {
    public:
        virtual ~ISong();
        bool operator==(const ISong &rhs) const;
        
        const uint32_t durationMillis() const;
        const string album() const;
        const string albumArtist() const;
        const string artist() const;
        string title() const;
        const uint32_t playCount() const;
        const uint64_t lastPlayed() const;
        const uint64_t creationDate() const;
        const uint32_t track() const;
        const uint32_t year() const;
        const int rating() const;
        const float position() const;
        const string albumArtUrl() const;
        const string albumId() const;
        const string artistId() const;
        virtual void setStringForKey(const string &key, const string &value) = 0;
        virtual const string uniqueId() const = 0;
        virtual string stringForKey(const string &key) const = 0;
        virtual const float floatForKey(const string &key) const = 0;
        virtual int64_t intForKey(const string &key) const = 0;
        virtual void setIntForKey(const string &key, const int64_t &value) = 0;
        // these 2 are here for compatibility, but ultimately these are _signed_ ints, because of sqlite
        uint64_t uIntForKey(const string &key) const;
        void setUintForKey(const string &key, const uint64_t &value);

        virtual void setFloatForKey(const string &key, const float &value) = 0;
        shared_ptr<OfflineState> offlineState() const;
        
        void setPlayCount(const uint32_t &value);
        void setLastPlayed(const uint64_t &value);
        void setPosition(float position);
        
        virtual bool editable() const;
        bool editable(const string &key) const;
        
        virtual void save() = 0;
        virtual shared_ptr<ISession> session() const = 0;
        
        // this method will block so it's best to call it synchronously
        virtual shared_ptr<IPlaybackData> playbackDataSync() = 0;
        virtual bool deepAsymmetricEquals(const ISong &rhs) const;
        virtual void freeUpMemory() const;
        
    private:
        // only permitted subclass is SongEntrySong
        ISong();

        ISong(const ISong &rhs); // delete
        ISong &operator=(const ISong &rhs); // delete

        // must be only called from player, because arrays must be updated
        void setRating(int rating);

        friend class SongEntrySong;
        friend class PlayerBase;
    };
    
    std::size_t hash_value(const shared_ptr<ISong> &rhs);
    bool operator==(const shared_ptr<ISong> &lhs, const shared_ptr<ISong> &rhs);
}

#endif
