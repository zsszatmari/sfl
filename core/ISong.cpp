//
//  ISong.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/25/13.
//
//

#include "ISong.h"
#include "OfflineStorage.h"
#include "BaseUtility.h"
#include "GearUtility.h"
#include HASH_H

namespace Gear
{
    using namespace Base;
    
#define method ISong::
    
    method ISong()
    {
    }
    
    method ~ISong()
    {
    }
    
    bool method operator==(const ISong &rhs) const
    {
        auto lMatch = stringForKey("matchedKey");
        auto rMatch = rhs.stringForKey("matchedKey");
        return (lMatch == rMatch && !lMatch.empty()) || uniqueId() == rhs.uniqueId();
    }
    
    const uint32_t method durationMillis() const
    {
        return (uint32_t)uIntForKey("durationMillis");
    }
    
    const string method albumId() const
    {
        return stringForKey("albumId");
    }
    
    const string method artistId() const
    {
        return stringForKey("artistId");
    }
    
    const string method album() const
    {
        return stringForKey("album");
    }
    
    const string method albumArtist() const
    {
        return stringForKey("albumArtist");
    }
    
    const string method artist() const
    {
        return stringForKey("artist");
    }
    
    string method title() const
    {
        return stringForKey("title");
    }
    
    const uint32_t method playCount() const
    {
        return (uint32_t)uIntForKey("playCount");
    }
    
    const uint64_t method lastPlayed() const
    {
        return uIntForKey("lastPlayed");
    }
    
    const uint64_t method creationDate() const
    {
        return uIntForKey("creationDate");
    }
    
    const uint32_t method track() const
    {
        return (uint32_t)uIntForKey("track");
    }
    
    const uint32_t method year() const
    {
        return (uint32_t)uIntForKey("year");
    }
    
    const int method rating() const
    {
        return (uint32_t)uIntForKey("rating");
    }
    
    const float method position() const
    {
        return floatForKey("position");
    }
    
    const string method albumArtUrl() const
    {
        return stringForKey("albumArtUrl");
    }
    
    std::size_t hash_value(const shared_ptr<ISong> &rhs)
    {
        HASH_NS::hash<std::string> hash;
        return hash(rhs->uniqueId());
    }
    
    bool operator==(const shared_ptr<ISong> &lhs, const shared_ptr<ISong> &rhs)
    {
        return equals(lhs, rhs);
    }
    
    bool method editable() const
    {
        return false;
    }
    
    bool method editable(const string &key) const
    {
        static const string e[] = {"album", "artist", "albumArtist", "title", "year", "genre", "track", "disc"};
        static std::vector<string> editableColumns = init<std::vector<string>>(e);
        return (editable() && find(editableColumns.begin(), editableColumns.end(), key) != editableColumns.end());
    }

    void method setPlayCount(const uint32_t &value)
    {
        setUintForKey("playCount", value);
    }
    
    void method setLastPlayed(const uint64_t &value)
    {
        setUintForKey("lastPlayed", value);
    }
    
    void method setRating(int rating)
    {
        setUintForKey("rating", rating);
    }
    
    void method setPosition(float position)
    {
        setFloatForKey("position", position);
    }
    
    bool method deepAsymmetricEquals(const ISong &rhs) const
    {
        return false;
    }
    
    void method freeUpMemory() const
    {
    }
    
    shared_ptr<OfflineState> method offlineState() const
    {
        return OfflineStorage::instance().state(*this);
    }

    void method setUintForKey(const string &key, const uint64_t &value)
    {
        setIntForKey(key, value);
    }

    uint64_t method uIntForKey(const string &key) const
    {
        return intForKey(key);
    }

    void method updateInDbAndRemote(const vector<string> &fields)
    {
        #pragma message("TODO: update song remotely")
        updateInDb(fields);
    }

    void method saveForTesting()
    {
        assert(false);
    }
}