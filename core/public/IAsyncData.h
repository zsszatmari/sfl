//
//  IAsyncData.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/20/13.
//
//

#ifndef __G_Ear__IAsyncData__
#define __G_Ear__IAsyncData__

#include <map>
#include <vector>
#include "stdplus.h"
#include "Thread.h"
#include MUTEX_H

namespace Gear
{
    using std::vector;
    using std::pair;

    class IAsyncData
    {
    public:
        IAsyncData();
        virtual ~IAsyncData();

        virtual void waitAsync(int offset, const std::function<void()> &f);
        void waitSync(int offset);
        virtual int offsetAvailable(int offset) const = 0;

        virtual void accessChunk(int offset, const std::function<void(const char *ptr, int available)> &f) = 0;
            
        virtual int totalLength() const = 0;
        virtual bool finished() const = 0;
        virtual int failed() const = 0;
        
        // this is undefined for multi-part downloads, as we don't retain whole file
        //void waitUntilFinished();
        
    protected:
        void signal();
        
    private:
        THREAD_NS::mutex waitingMutex;
        vector<pair<int, std::function<void()>>> waiting;
    };
}

#endif /* defined(__G_Ear__IAsyncData__) */
