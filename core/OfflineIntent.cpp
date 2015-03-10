//
//  OfflineIntent.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 29/01/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "OfflineIntent.h"
#include "OfflineStorage.h"
#include "BackgroundExecutor.h"
#include "SongEntry.h"
#include "Iplaylist.h"
#include "OfflineService.h"
#include "ISession.h"
#include "IoService.h"

namespace Gear
{
#define method OfflineIntent::
    
    shared_ptr<OfflineIntent> method create(const vector<SongEntry> &songs)
    {
        // currently youtube songs are not permitted
        vector<SongEntry> filteredSongs;
        remove_copy_if(songs.begin(), songs.end(), back_inserter(filteredSongs), [](const SongEntry &entry){
            auto session = entry.song()->session();
            return !session || !session->saveForOfflinePossible();
        });
        if (filteredSongs.empty()) {
            return nullptr;
        }
        
        if (OfflineService::available()) {
            return shared_ptr<OfflineIntent>(new OfflineIntent(filteredSongs));
        } else {
            return nullptr;
        }
    }
    
    method OfflineIntent(const vector<SongEntry> &songs)
    {
        _download = false;
        // if one of them isn't available for offline, go for the download
        for (auto &song : songs) {
            if (!OfflineStorage::instance().available(*song.song())) {
                _download = true;
                break;
            }
        }
    }
    
    std::string method menuText(bool download)
    {
        if (download) {
            return "Download for offline";
        } else {
            return "Remove offline copy";
        }
    }
    
    const string method menuText() const
    {
        return menuText(_download);
    }
    
    void method apply(const vector<SongEntry> &songs, bool download)
    {
        Io::get().dispatch([=]{
            
            if (download) {
                for (auto &song : songs) {
                    OfflineStorage::instance().store(song.song()->uniqueId(), song.song());
                }
            } else {
                for (auto &song : songs) {
                    OfflineStorage::instance().remove(song.song()->uniqueId(), song.song());
                }
            }
        });
    }
    
    void method apply(const vector<SongEntry> &songs) const
    {
        apply(songs, _download);
    }
}
