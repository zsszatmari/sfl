//
//  IPlaybackData.cpp
//  G-Ear Player
//
//  Created by zsoltt on 10/15/13.
//
//

#include "IPlaybackData.h"

namespace Gear
{
#define method IPlaybackData::
    
    method IPlaybackData(const IPlaybackData::Format format) :
        _format(format)
    {
    }
    
    IPlaybackData::Format method format() const
    {
        return _format;
    }
}
