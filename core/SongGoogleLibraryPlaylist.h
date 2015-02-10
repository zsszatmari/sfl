//
//  SongGoogleLibraryPlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/17/13.
//
//

#ifndef __G_Ear_Player__SongGoogleLibraryPlaylist__
#define __G_Ear_Player__SongGoogleLibraryPlaylist__

#include "SongLibraryPlaylist.h"
#include "GooglePlaySession.h"

namespace Gear
{
    class SongGoogleLibraryPlaylist final : public SongLibraryPlaylist
    {
    public:
        static shared_ptr<SongGoogleLibraryPlaylist> create(const shared_ptr<GooglePlaySession> &session);
    
        virtual void removeSongs(const vector<SongEntry> &songs);
        
        virtual bool saveForOfflinePossible() const;
        
    private:
        SongGoogleLibraryPlaylist(const shared_ptr<GooglePlaySession> &session);
    };
}

#endif /* defined(__G_Ear_Player__SongGoogleLibraryPlaylist__) */
