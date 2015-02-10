//
//  IDecoder.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/19/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__IDecoder__
#define __G_Ear_core__IDecoder__

#include <cstddef>
#include <stdint.h>

namespace Gear
{
    class IDecoder
    {
    public:
        enum class Status
        {
            NoError = 0,
            NeedMore = 1,
            End = 2,
            UnknownError = 3
        };
        
        virtual ~IDecoder();
        virtual Status readInto(unsigned char *outData, size_t outSize,size_t &producedBytes) = 0;
        virtual void feedFrom(unsigned const char *inData, size_t inSize) = 0;
        virtual long seekOffset(float time, size_t guess) = 0;
        virtual void setDiscontinous() = 0;
        virtual void setTotalLength(int64_t totalLength) {}
    };
};

#endif /* defined(__G_Ear_core__IDecoder__) */
