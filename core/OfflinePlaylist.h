//
//  OfflinePlaylist.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 31/03/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__OfflinePlaylist__
#define __G_Ear_core__OfflinePlaylist__

#include "ModifiablePlaylist.h"

namespace Gear
{
    class OfflinePlaylist final : public StoredPlaylist
    {
    public:
        static shared_ptr<OfflinePlaylist> create(const string &playlistId, const string &name,  const shared_ptr<ISession> &session);
        
        vector<shared_ptr<IPlaylistIntent>> playlistIntents();
        
    private:
        OfflinePlaylist(const string &playlistId, const string &name, const shared_ptr<ISession> &session);
        
        virtual void addSongEntries(const vector<SongEntry> &songs);
        virtual void removeSongs(const vector<SongEntry> &songs);
        virtual bool moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before);
        virtual void remove();
    };
}

#endif /* defined(__G_Ear_core__OfflinePlaylist__) */
