//
//  QueueIntent.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 29/04/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "QueueIntent.h"
#include "QueueSongArray.h"

namespace Gear
{
#define method QueueIntent::
    
    method QueueIntent(const Flavour flavour) :
        _flavour(flavour)
    {
    }
    
    const string method menuText() const
    {
        switch(_flavour) {
            case Flavour::EnqueueLast:
                return "Add to Queue";
            case Flavour::EnqueueNext:
                return "Play Next";
            case Flavour::Remove:
                return "Remove From Queue";
        }
    }
    
    void method apply(const vector<SongEntry> &songs) const
    {
        auto queue = QueueSongArray::instance();
        switch(_flavour) {
            case Flavour::EnqueueLast:
                queue->enqueue(songs, false);
                break;
            case Flavour::EnqueueNext:
                queue->enqueue(songs, true);
                break;
            case Flavour::Remove:
                queue->removeFromQueue(songs);
                break;

        }
    }
}