//
//  LastFmService.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/21/13.
//
//

#ifndef __G_Ear_Player__LastFmService__
#define __G_Ear_Player__LastFmService__

#include <string>
#include "stdplus.h"
#include MEMORY_H
#include WEAK_H
#include "ImageToDeliver.h"

namespace Gear
{
    using std::string;

    class ISong;
    
    class LastFmService final
    {
    public:
        static std::pair<string,ImageToDeliver::Rating> getAlbumArtUrlForArtist(const string &artist, const string &album, const string &title, int preferredSize, weak_ptr<void> stillNeeded);
        static void playTimePassed(float duration);
        static void songBegan(const MEMORY_NS::shared_ptr<ISong> &song);
    };
}

#endif /* defined(__G_Ear_Player__LastFmService__) */
