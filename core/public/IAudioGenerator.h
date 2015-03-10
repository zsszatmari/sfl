//
//  IAudioGenerator.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 16/05/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__IAudioGenerator__
#define __G_Ear_core__IAudioGenerator__

#include <stdint.h>

namespace Gear
{
    class IAudioGenerator
    {
    public:
        virtual ~IAudioGenerator();
        virtual int getUncompressedDataInto(char *dataPtr, int length, const uint8_t bufferId) = 0;
        virtual void silence(uint8_t bufferId);
    };
}

#endif /* defined(__G_Ear_core__IAudioGenerator__) */
