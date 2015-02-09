//
//  IPlaybackData.h
//  G-Ear Player
//
//  Created by zsoltt on 10/15/13.
//
//

#ifndef __G_Ear_Player__IPlaybackData__
#define __G_Ear_Player__IPlaybackData__

#include "IAsyncData.h"

namespace Gear
{
    class IPlaybackData : public IAsyncData
    {
    public:
        enum class Format {
            Mp3,
            Mp4,
            PcmSignedInt16
        };
        
        IPlaybackData(const Format format);
        
        Format format() const;
        virtual void seek(int offset) = 0;
        
    private:
        Format _format;
    };
}

#endif /* defined(__G_Ear_Player__IPlaybackData__) */
