//
//  OfflineSongArray.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 01/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "OfflineSongArray.h"
#include "OfflineStorage.h"
#include "OfflineIntent.h"
#include "OfflineService.h"
#include "IApp.h"
#include "Db.h"

namespace Gear
{
#define method OfflineSongArray::
    
    shared_ptr<OfflineSongArray> method create(const shared_ptr<ISession> &session)
    {
        auto ret = shared_ptr<OfflineSongArray>(new OfflineSongArray(session));
        return ret;
    }
    
    method OfflineSongArray(const shared_ptr<ISession> &session) :
        SortedSongArray(IApp::instance()->db(), ClientDb::Predicate(ClientDb::Predicate::Operator::Equals, "savedForOffline", true)),
        _session(session)
    {
        _refreshCount = 0;
    }
    
    void method refresh()
    {
        auto s = _session.lock();
        std::vector<std::string> playlistNames;
        if (s) {
            auto pThis = shared_from_this();
            ++_refreshCount;
            _executor.addTask([pThis,s]{
                
                if (pThis->_refreshCount == 1) {
                
                    vector<SongEntry> entries = OfflineStorage::instance().allEntries(s);
#ifdef DEBUG
                    /*    std::cout << "offline entries:\n";
                     for (const auto &entry : entries) {
                     std::cout << (std::string)entry.fastTitle() << std::endl;
                     }*/
#endif
                    
                    #pragma message("TODO: displaying offline songs is unfinished...")
                    //pThis->setAllSongs(entries, false, true);
                }
                
                --pThis->_refreshCount;
            });
        }
    }
    
    vector<shared_ptr<ISongIntent>> method songIntents(const std::vector<SongEntry> &songs, const std::string &fieldHint) const
    {
        vector<shared_ptr<ISongIntent>> ret;
        for (auto &song : songs) {
            // if the songs are not yet available...
            if (!OfflineStorage::instance().available(song.song()->uniqueId())) {
                return ret;
            }
        }
        if (OfflineService::available()) {
            auto intent = OfflineIntent::create(songs);
            if (intent) {
                ret.push_back(intent);
            }
        }
        return ret;
    }
}
