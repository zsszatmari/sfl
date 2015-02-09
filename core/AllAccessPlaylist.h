//
//  AllAccessPlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__AllAccessPlaylist__
#define __G_Ear_Player__AllAccessPlaylist__

#include "BasicPlaylist.h"

namespace Gear
{
    class ISession;
    
    class AllAccessPlaylist final : public BasicPlaylist
    {
    public:
        AllAccessPlaylist(const shared_ptr<ISession> &session);
        
        virtual bool saveForOfflinePossible() const;
    };
}

#endif /* defined(__G_Ear_Player__AllAccessPlaylist__) */
