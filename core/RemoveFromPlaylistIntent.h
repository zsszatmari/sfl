//
//  RemoveFromPlaylistIntent.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/17/13.
//
//

#ifndef __G_Ear_Player__RemoveFromPlaylistIntent__
#define __G_Ear_Player__RemoveFromPlaylistIntent__

#include "ISongIntent.h"
#include "IPlaylist.h"

namespace Gear
{
    class RemoveFromPlaylistIntent final : public ISongIntent
    {
    public:
        // don't need info about concrete playlist here. SongEntry will know it anyway
        RemoveFromPlaylistIntent(const shared_ptr<ISession> &session);
        
        virtual const string menuText() const;
        virtual void apply(const vector<SongEntry> &songs) const;
        virtual bool confirmationNeeded(const vector<SongEntry> &songs) const;
        virtual string confirmationText(const vector<SongEntry> &songs) const;
        
    private:
        const weak_ptr<ISession> _session;
    };
}

#endif /* defined(__G_Ear_Player__RemoveFromPlaylistIntent__) */
