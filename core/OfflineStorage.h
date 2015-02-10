//
//  OfflineStorage.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 29/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__OfflineStorage__
#define __G_Ear_core__OfflineStorage__

#include <map>
#include "SerialExecutor.h"
#include "EventSignal.h"
#include "IPlaybackData.h"
#include UNORDERED_MAP_H

namespace Gear
{
    class IPlaybackData;
    class OfflineState;
    class ISong;
    class SongEntry;
    class ISession;
    class IPlaylist;
    class OfflinePlaylist;
    
    class OfflineStorage final
    {
    public:
        static OfflineStorage &instance();
        void store(const std::string &identifier, const shared_ptr<ISong> &song);
        void remove(const std::string &identifier);
        void storePlaylist(const std::string &name, const std::vector<std::string> &songIds);
        void removePlaylist(const std::string &name);
        std::vector<shared_ptr<IPlaylist>> playlists(const shared_ptr<ISession> session);
        
        shared_ptr<IPlaybackData> fetch(const std::string &identifier, const IPlaybackData::Format format) const;
        // if returns true, but ratio is under 1.0f, it cannot be fetched:
        bool available(const std::string &identifier, float &ratio) const;
        // if returns true, it can be fetched:
        bool available(const std::string &identifier) const;
        shared_ptr<OfflineState> state(const std::string &identifier);
        std::vector<SongEntry> allEntries(const shared_ptr<ISession> &session) const;
        Base::EventConnector updatedEvent();
        
        static void decode(char *data, int available, int offset);
        
        static const std::string SourceSessionKey;
        
    private:
        OfflineStorage();
        OfflineStorage(const OfflineStorage &); // delete
        OfflineStorage &operator=(const OfflineStorage &); // delete

        static void encode(const char *data, int available, int offset, std::vector<char> &result);
        
        bool availableUnsafe(const std::string &identifier, float &ratio) const;
        void update(const std::string &identifier, float value);
        bool doStore(const std::string &identifier, const shared_ptr<IPlaybackData> &data);
        bool doStoreMeta(const std::string &identifier, const std::string &metaString);
        
        Base::SerialExecutor _executor;
        Base::EventSignal _updatedEvent;
        
        std::map<std::string, std::pair<float,bool>> _items;
        UNORDERED_NS::unordered_map<std::string, weak_ptr<OfflineState>> _itemStates;
        std::vector<shared_ptr<OfflinePlaylist>> _playlists;
        mutable THREAD_NS::mutex _itemsMutex;
    };
}

#endif /* defined(__G_Ear_core__OfflineStorage__) */
