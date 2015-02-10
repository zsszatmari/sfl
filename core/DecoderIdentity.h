//
//  DecoderIdentity.h
//  G-Ear Player
//
//  Created by zsoltt on 10/15/13.
//
//

#ifndef __G_Ear_Player__DecoderIdentity__
#define __G_Ear_Player__DecoderIdentity__

#include "IDecoder.h"
#include "CircularBuffer.h"

namespace Gear
{
    class DecoderIdentity final : public IDecoder
    {
    public:
        DecoderIdentity();
        
        virtual Status readInto(unsigned char *outData, size_t outSize,size_t &producedBytes);
        virtual void feedFrom(unsigned const char *inData, size_t inSize);
        virtual long seekOffset(float time, size_t guess);
        virtual void setDiscontinous();
        
    private:
        CircularBuffer _buffer;
    };
}

#endif /* defined(__G_Ear_Player__DecoderIdentity__) */
