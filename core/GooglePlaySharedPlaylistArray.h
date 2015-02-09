//
//  GooglePlaySharedPlaylistArray.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/20/13.
//
//

#ifndef __G_Ear_Player__GooglePlaySharedPlaylistArray__
#define __G_Ear_Player__GooglePlaySharedPlaylistArray__

#include "GooglePlayNonLocalArray.h"

namespace Gear
{
    class GooglePlaySharedPlaylistArray final : public GooglePlayNonLocalArray, public MEMORY_NS::enable_shared_from_this<GooglePlaySharedPlaylistArray>
    {
    public:
        static shared_ptr<GooglePlaySharedPlaylistArray> create(const string &playlistId,const shared_ptr<GooglePlaySession> &session);
        
        
    private:
        GooglePlaySharedPlaylistArray(const string &playlistId, const shared_ptr<GooglePlaySession> &session);
        
        void fetch();
        
        const string _playlistId;
    };
}

#endif /* defined(__G_Ear_Player__GooglePlaySharedPlaylistArray__) */
