//
//  PlaybackItem.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/28/12.
//
//

#ifndef PLAYBACKITEMDELEGATE_H
#define PLAYBACKITEMDELEGATE_H

namespace Gear
{
    class PlaybackItem;
    
    class PlaybackItemDelegate
    {
    public:
        virtual ~PlaybackItemDelegate() {}
        virtual void rateChangedForPlaybackItem(const PlaybackItem &item, float rate) = 0;
    };
}

#endif
