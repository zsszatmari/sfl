//
//  OfflinePlaylist.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 31/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "OfflinePlaylist.h"
#include "OfflinePlaylistIntent.h"

namespace Gear
{
#define method OfflinePlaylist::
    
    shared_ptr<OfflinePlaylist> method create(const string &playlistId, const string &name, const shared_ptr<ISession> &session)
    {
        auto playlist = shared_ptr<OfflinePlaylist>(new OfflinePlaylist(playlistId, name, session));
        playlist->init();
        return playlist;
    }
    
    method OfflinePlaylist(const string &playlistId, const string &name, const shared_ptr<ISession> &session) :
        StoredPlaylist(playlistId, name, session)
    {
    }
    
    vector<shared_ptr<IPlaylistIntent>> method playlistIntents()
    {
        vector<shared_ptr<IPlaylistIntent>> ret;
        ret.push_back(shared_ptr<IPlaylistIntent>(new OfflinePlaylistIntent(songArray(), false, name())));
        return ret;
    }
    
    void method addSongEntries(const vector<SongEntry> &songs)
    {
    }
    
    void method removeSongs(const vector<SongEntry> &songs)
    {
    }
    
    bool method moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before)
    {
        return false;
    }
    
    void method remove()
    {
    }
}
