//
//  CircularBuffer.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/19/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <stdlib.h>
#include <iostream>
#ifdef __APPLE__
#include <mach/mach.h>
#endif
#include "CircularBuffer.h"


#if defined(CIRCULARBUFFER_MEMCPY_WRAP_PRODUCE) || defined(CIRCULARBUFFER_MEMCPY_WRAP_CONSUME)
#define MULTIPLIER 2
#else
#define MULTIPLIER 1
#endif

#ifdef __APPLE__
// based on TPCircularBuffer
static void *circularalloc(int &length) {

    // Keep trying until we get our buffer, needed to handle race conditions
    //int retries = 3;
    while ( true ) {

        length = (int32_t)round_page(length);    // We need whole page sizes

        // Temporarily allocate twice the length, so we have the contiguous address space to
        // support a second instance of the buffer directly after
        vm_address_t bufferAddress;
        kern_return_t result = vm_allocate(mach_task_self(),
                                           &bufferAddress,
                                           length * 2,
                                           VM_FLAGS_ANYWHERE); // allocate anywhere it'll fit
        if ( result != ERR_SUCCESS ) {
            /*if ( retries-- == 0 ) {
                reportResult(result, "Buffer allocation");
                return false;
            }*/
            // Try again if we fail
            continue;
        }
        
        // Now replace the second half of the allocation with a virtual copy of the first half. Deallocate the second half...
        result = vm_deallocate(mach_task_self(),
                               bufferAddress + length,
                               length);
        if ( result != ERR_SUCCESS ) {
            /*if ( retries-- == 0 ) {
                reportResult(result, "Buffer deallocation");
                return false;
            }*/
            // If this fails somehow, deallocate the whole region and try again
            vm_deallocate(mach_task_self(), bufferAddress, length);
            continue;
        }
        
        // Re-map the buffer to the address space immediately after the buffer
        vm_address_t virtualAddress = bufferAddress + length;
        vm_prot_t cur_prot, max_prot;
        result = vm_remap(mach_task_self(),
                          &virtualAddress,   // mirror target
                          length,    // size of mirror
                          0,                 // auto alignment
                          0,                 // force remapping to virtualAddress
                          mach_task_self(),  // same task
                          bufferAddress,     // mirror source
                          0,                 // MAP READ-WRITE, NOT COPY
                          &cur_prot,         // unused protection struct
                          &max_prot,         // unused protection struct
                          VM_INHERIT_DEFAULT);
        if ( result != ERR_SUCCESS ) {
            /*if ( retries-- == 0 ) {
                reportResult(result, "Remap buffer memory");
                return false;
            }*/
            // If this remap failed, we hit a race condition, so deallocate and try again
            vm_deallocate(mach_task_self(), bufferAddress, length);
            continue;
        }
        
        if ( virtualAddress != bufferAddress+length ) {
            // If the memory is not contiguous, clean up both allocated buffers and try again
            /*if ( retries-- == 0 ) {
                printf("Couldn't map buffer memory to end of buffer\n");
                return false;
            }*/

            vm_deallocate(mach_task_self(), virtualAddress, length);
            vm_deallocate(mach_task_self(), bufferAddress, length);
            continue;
        }

        return (void*)bufferAddress;
    }
}

static void circularfree(void *buf, int length)
{
    vm_deallocate(mach_task_self(), (vm_address_t)buf, length * 2);
}
#endif

namespace Gear
{
#define method CircularBuffer::
    
    method CircularBuffer(int length) :
        _tail(0),
        _head(0),
        _length(length),
        _buffer(malloc(length * MULTIPLIER)),
        _noNeedToCopyUpper(false)
    {
		_fillCount = 0;
#if DEBUG
        std::cout << "allocating circular buffer with multiplier of " << MULTIPLIER << std::endl;
#endif
#ifdef __APPLE__
        _buffer = circularalloc(_length);
#else
        _buffer = malloc(length * MULTIPLIER);
#endif
    }
    
    method ~CircularBuffer()
    {
#ifdef __APPLE__
        circularfree(_buffer, _length);
#else
        free(_buffer);
#endif
    }
}
