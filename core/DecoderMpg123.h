//
//  DecoderMpg123.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/22/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__DecoderMpg123__
#define __G_Ear_core__DecoderMpg123__

// declare before mpg123.h for ssize_t
#include "Environment.h"
#define __STDC_CONSTANT_MACROS
extern "C" {
#include <libavresample/avresample.h>
#include <mpg123.h>
}
#include <vector>
#include "IDecoder.h"

namespace Gear
{

    class DecoderMpg123 final : public IDecoder
    {
    public:
        DecoderMpg123();
        virtual ~DecoderMpg123();
        
        virtual IDecoder::Status readInto(unsigned char *outData, size_t outSize,size_t &producedBytes);
        virtual void feedFrom(unsigned const char *inData, size_t inSize);
        virtual long seekOffset(float time, size_t guess);
        virtual void setDiscontinous();
        
    private:
        long _bitrate;
        mpg123_handle *_mpgHandle;
        AVAudioResampleContext *_resample;
        std::vector<unsigned char> _resampleData;
        std::vector<unsigned char> _extraBuffer;
        size_t _extraBufferPos;
    };
}

#endif /* defined(__G_Ear_core__DecoderMpg123__) */
