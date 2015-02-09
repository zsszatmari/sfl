//
//  GooglePlayThumbsPlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/7/13.
//
//

#ifndef __G_Ear_Player__GooglePlayThumbsPlaylist__
#define __G_Ear_Player__GooglePlayThumbsPlaylist__

#include "IPlaylist.h"

namespace Gear
{
    class GooglePlaySession;
    
    class GooglePlayThumbsPlaylist : public IPlaylist
    {
    public:
        GooglePlayThumbsPlaylist(const shared_ptr<GooglePlaySession> &session);
        
        virtual const shared_ptr<ISongArray> songArray();
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const string playlistId() const;
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        virtual bool saveForOfflinePossible() const;
        void reset();
        
    private:
        weak_ptr<ISongArray> _songArray;
        weak_ptr<GooglePlaySession> _session;
    };
}

#endif /* defined(__G_Ear_Player__GooglePlayThumbsPlaylist__) */
