//
//  CreatePlaylistIntent.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#ifndef __G_Ear_Player__CreatePlaylistIntent__
#define __G_Ear_Player__CreatePlaylistIntent__

#include "ISongNoIndexIntent.h"
#include "GooglePlaySession.h"

namespace Gear
{
    class CreatePlaylistIntent final : public ISongNoIndexIntent
    {
    public:
        CreatePlaylistIntent(const shared_ptr<PlaylistSession> &session);
    
        virtual const string menuText() const;
        virtual void apply(const vector<shared_ptr<ISong>> &songs) const;
        
    private:
        const shared_ptr<PlaylistSession> _session;
    };
}


#endif /* defined(__G_Ear_Player__CreatePlaylistIntent__) */
