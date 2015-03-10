//
//  DecoderIdentity.cpp
//  G-Ear Player
//
//  Created by zsoltt on 10/15/13.
//
//

#include "DecoderIdentity.h"

namespace Gear
{
#define method DecoderIdentity::
    
    method DecoderIdentity() :
        _buffer(65536)
    {
    }
    
    IDecoder::Status method readInto(unsigned char *outData, size_t outSize,size_t &producedBytes)
    {
        int available = 0;
        void *data = _buffer.tail(available);
        if (!data) {
            producedBytes = 0;
            return IDecoder::Status::NeedMore;
        }
        
        if (available < outSize) {
            producedBytes = available;
            memcpy(outData, data, available);
            _buffer.consume(available);
            return IDecoder::Status::NeedMore;
        } else {
            memcpy(outData, data, outSize);
            _buffer.consume((int)outSize);
            return IDecoder::Status::NoError;
        }
    }
    
    void method feedFrom(unsigned const char *inData, size_t inSize)
    {
        int freeSpace = 0;
        void *freeMem = _buffer.head(freeSpace);
        if (freeSpace < inSize) {
            inSize = freeSpace;
        }
        memcpy(freeMem, inData, inSize);
        _buffer.produce((int)inSize);
    }
    
    long method seekOffset(float time, size_t guess)
    {
        return guess;
    }
    
    void method setDiscontinous()
    {
    }
}
