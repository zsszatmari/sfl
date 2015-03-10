//
//  ISongEntry.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/20/13.
//
//

#include "SongEntry.h"
#include "SongEntrySong.h"
#include "json.h"
#include HASH_H

namespace Gear
{
#define method SongEntry::
    
    method SongEntry(const SongData &value) :
        _song(new SongEntrySong())
    {
        _song->_values = value;
    }

    method SongEntry(const string &source, const string &playlist, const string &entryId, const string &songId) :
        _song(new SongEntrySong())
    {
        #pragma message("TODO: create or update a song in the db (not here!)")
        _song->setStringForKey("entryId", entryId);
        _song->setStringForKey("id", songId);
        _song->setStringForKey("source", source);
        _song->setStringForKey("playlist", playlist);
    }
        
    method SongEntry(const shared_ptr<ISong> &song, const string &entryId) :
        _song(new SongEntrySong(song))
    {
        _song->setStringForKey("entryId", entryId);
    }

    method SongEntry() :  
        _song(new SongEntrySong())
    {
    }

    /*method SongEntry(const SongEntry &rhs) 
    {
        std::cout << "rhs: " << &rhs << std::endl;

        _song = (rhs._song);
        _entryId = (rhs._entryId);
        _fastTitle = (rhs._fastTitle);
    }*/

    SongEntry method clone(int16_t replicaDiff) const
    {
        SongEntry ret(song(),entryId());
        ret.song()->setIntForKey("replica", song()->intForKey("replica") + replicaDiff);
        //ret._replica += replicaDiff;
        return ret;
    }

    SongEntry method cloneWith(int16_t replica) const
    {
        SongEntry ret(song(),entryId());
        ret.song()->setIntForKey("replica", replica);
        //ret._replica = replica;
        return ret;
    }

    int16_t method replica() const
    {
        return song()->intForKey("replica");
    }

    SongEntry method unclone() const
    {
        SongEntry ret(song(),entryId());
        ret.song()->setIntForKey("replica", 0);
        return ret;
    }
    
    const shared_ptr<ISong> method song() const
    {
        return _song;
    }
    
    string method entryId() const
    {
        return _song->stringForKey("entryId");
    }
    
    bool method operator==(const SongEntry &rhs) const
    {
        //std::cout << "==? " << _entryId << " " << rhs._entryId << std::endl;
#ifdef _DEBUG
    	//AppLog("compare '%s' ... '%s'", _entryId.c_str(), rhs._entryId.c_str());
#endif
    	return replica() == rhs.replica() && entryId() == rhs.entryId(); 
    }
    
    bool method operator!=(const SongEntry &rhs) const
    {
        return !operator==(rhs);
    }
    
    bool method operator<(const SongEntry &rhs) const
    {
        if (replica() < rhs.replica()) {
            return true;
        } else if (replica() > rhs.replica()) {
            return false;
        }

        return entryId() < rhs.entryId();
    }
    
    method operator bool() const
    {
        return !entryId().empty();
    }
    
    std::size_t hash_value(const SongEntry &rhs)
    {
        return HASH_NS::hash<std::string>()(rhs.entryId());
    }
    
    void swap(Gear::SongEntry &lhs, Gear::SongEntry &rhs)
    {
        swap(lhs._song, rhs._song);
    }

    Json::Value method toJson() const
    {
        return _song->_values.toJson();
    }
}
