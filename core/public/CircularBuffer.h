//
//  CircularBuffer.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/19/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__CircularBuffer__
#define __G_Ear_core__CircularBuffer__

#include <stdint.h>
#include <string.h>
#include <cassert>
#include "stdplus.h"
#include ATOMIC_H

//#define CIRCULARBUFFER_MEMCPY_WRAP_PRODUCE

// not yet understand why is this necessary... but causes skips otherwise!
#ifndef __APPLE__
#pragma message("check circularbuffer for playback")
#define CIRCULARBUFFER_MEMCPY_WRAP_CONSUME
#else
#define CIRCULAR_VM
#endif

namespace Base
{
}

namespace Gear
{
    using namespace Base;    
    
    class CircularBuffer
    {
    public:
        CircularBuffer(int length);
        ~CircularBuffer();
        
        // for receiving new data
        void *head(int &availableBytes)
        {
            availableBytes = (_length - _fillCount);
            if (availableBytes == 0 ) return nullptr;
            
#ifndef CIRCULARBUFFER_MEMCPY_WRAP_PRODUCE
#ifndef CIRCULAR_VM
            auto maxBytes = (_length - _head);
            if (availableBytes > maxBytes) {
                availableBytes = maxBytes;
            }
#endif
#endif
            
            return (void*)((char*)_buffer + _head);
        }

        void headTotal(int &availableBytes)
        {
        	// total amount receivable (but not necessarily writable at once)
        	availableBytes = (_length - _fillCount);
        }

        void produce(int amount)
        {
#ifndef CIRCULAR_VM
#ifdef CIRCULARBUFFER_MEMCPY_WRAP_PRODUCE
            if (_head + amount > _length) {
                memcpy(_buffer, (char *)_buffer+ _length, _head + amount - _length);
            }
#endif
            _noNeedToCopyUpper = false;
#endif
            
            _head = (_head + amount) % _length;
            _fillCount += amount;
#ifdef DEBUG
            int count = _fillCount;
            assert(count >= 0);
#endif
        }
        
        // for consuming data
        void *tail(int32_t &availableBytes)
        {
            availableBytes = _fillCount;
            if ( availableBytes == 0 ) return nullptr;
            
#ifndef CIRCULARBUFFER_MEMCPY_WRAP_CONSUME
#ifndef CIRCULAR_VM
            auto maxBytes = _length - _tail;
            if ( availableBytes > maxBytes) {
                availableBytes = maxBytes;
            }
#endif
#else
            if (_tail + availableBytes > _length && !_noNeedToCopyUpper) {
                memcpy((char *)_buffer + _length, (char *)_buffer, _tail + availableBytes - _length);
                _noNeedToCopyUpper = true;
            }
#endif
            
            return (void*)((char*)_buffer + _tail);
        }
        
        void tailTotal(int32_t &availableBytes)
        {
        	availableBytes = _fillCount;
        }

        void consume(int32_t amount)
        {
            _tail = (_tail + amount) % _length;
            _fillCount += (-amount);
            
#ifdef DEBUG
            int count = _fillCount;
            assert(count >= 0);
#endif
        }
        
        void consumeAll()
        {
        	consume(_fillCount);
        }

    private:
        CircularBuffer(const CircularBuffer &rhs); // delete
        CircularBuffer &operator=(const CircularBuffer &rhs); // delete

        void *_buffer;
        int32_t _length;
        int32_t _tail;
        int32_t _head;
        ATOMIC_NS::atomic_int _fillCount;
        ATOMIC_NS::atomic<bool> _noNeedToCopyUpper;
    };
    
    template<int N>
    class CircularBufferSized : public CircularBuffer
    {
    public:
        CircularBufferSized() :
            CircularBuffer(N)
        {
        }
    };
}

#endif /* defined(__G_Ear_core__CircularBuffer__) */
