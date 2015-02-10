//
//  OfflinePlaylistIntent.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 01/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "OfflinePlaylistIntent.h"
#include "OfflineIntent.h"
#include "IPlaylist.h"
#include "ISongArray.h"
#include "ValidPtr.h"
#include "BackgroundExecutor.h"
#include "OfflineStorage.h"

namespace Gear
{
#define method OfflinePlaylistIntent::
    
    method OfflinePlaylistIntent(const shared_ptr<const ISongArray> &songArray, bool download, const std::string &name) :
        _download(download),
        _songArray(songArray),
        _name(name)
    {
    }
    
    const std::string method menuText() const
    {
        return OfflineIntent::menuText(_download);
    }
    
    const std::string method confirmationText() const
    {
        return "";
    }
    
    void method apply()
    {
        auto s = _songArray.lock();
        if (s) {
            #pragma message("TODO: can't download for offline")
            /*
            auto songsPtr = s->rawSongs();
            const vector<SongEntry> &songs = *songsPtr;
            if (_download) {
                
                OfflineIntent::apply(songs, _download);
            } else {
                // removePlaylist() does the remove of songs
                // on the other hand, if we don't want to save as a playlist (e.g. artist grouping):
                if (_name.empty()) {
                    OfflineIntent::apply(songs, _download);
                    return;
                }
            }
            std::vector<string> songIds;
            transform(songs.begin(), songs.end(), back_inserter(songIds), [](const SongEntry &entry){
                return entry.song()->uniqueId();
            });
            auto download = _download;
            auto name = _name;
            Base::BackgroundExecutor::instance().addTask([songIds,download,name]{
                
                if (download) {
                    OfflineStorage::instance().storePlaylist(name, songIds);
                } else {
                    OfflineStorage::instance().removePlaylist(name);
                }
            });*/
        }
    }
}
