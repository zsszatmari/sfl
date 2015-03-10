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
#include <functional>
#include "SerialExecutor.h"
#include "EventSignal.h"
#include "IPlaybackData.h"
#include UNORDERED_MAP_H
#include "sfl/Maybe.h"

namespace Gear
{
    using std::string;
    using std::function;

    class IPlaybackData;
    class OfflineState;
    class ISession;
    class Db;
    class ISong;
    
    class OfflineStorage final
    {
    public:
        static OfflineStorage &instance();
        void store(const std::string &identifier, const shared_ptr<ISong> &song);
        void remove(const std::string &identifier, const shared_ptr<ISong> &song);
        void storePlaylist(const std::string &name, const std::vector<std::string> &songIds);
        void removePlaylist(const std::string &name);
        
        shared_ptr<IPlaybackData> fetch(const std::string &identifier, const IPlaybackData::Format format) const;
        // if returns true, but ratio is under 1.0f, it cannot be fetched:
        //bool available(const std::string &identifier, float &ratio) const;
        // if returns true, it can be fetched:
        bool available(const ISong &identifier) const;
        shared_ptr<OfflineState> state(const ISong &identifier);
        Base::EventConnector updatedEvent();
        
        static void decode(char *data, int available, int offset);
        
        static void putLegacyInDb(const shared_ptr<Db> &db);

        
    private:
        OfflineStorage();
        OfflineStorage(const OfflineStorage &); // delete
        OfflineStorage &operator=(const OfflineStorage &); // delete

        static void encode(const char *data, int available, int offset, std::vector<char> &result);
        
        void update(const shared_ptr<ISong> &song, sfl::Maybe<float> ratio);

        void doStore(const string &identifier, const shared_ptr<ISong> &song, const shared_ptr<IPlaybackData> &data, const function<void(bool)> &result);

        Base::SerialExecutor _executor;
        Base::EventSignal _updatedEvent;
        
        std::map<std::string, weak_ptr<OfflineState>> _itemStates;
        mutable THREAD_NS::mutex _itemsMutex;
    };
}

#endif /* defined(__G_Ear_core__OfflineStorage__) */
