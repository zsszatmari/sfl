//
//  PlaybackData.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/17/13.
//
//

#ifndef __G_Ear__PlaybackData__
#define __G_Ear__PlaybackData__

#include <string>
#include <vector>
#include "stdplus.h"
#include "IPlaybackData.h"
#include "ISeekDownloader.h"

namespace Gear
{
    using std::string;
    using std::function;
    
    class PlaybackData final : public IPlaybackData
    {
    public:
        PlaybackData(const vector<string> &urls, const Format format, const IDownloaderFactory &downloaderFactory = DownloaderFactory());
        virtual ~PlaybackData();
        
        // returns 0 if not available
        virtual int offsetAvailable(int offset) const;
        virtual void waitAsync(int offset, const function<void()> &f);
        virtual void accessChunk(int offset, const function<void(const char *ptr, int available)> &f);
        virtual void seek(int offset);
        
        virtual int totalLength() const;
        virtual bool finished() const;
        virtual int failed() const;
        
        const string httpHeader() const;
        
    private:
        PlaybackData(const PlaybackData &rhs); // delete
        PlaybackData &operator=(const PlaybackData &rhs); // delete
        
        void startPrefetchingAtIndex(int index);
        const vector<pair<int,int>> &ranges() const;
        
        const vector<pair<int,int>> _ranges;
        const vector<long long> _waitTimes;
        // non-boost vector seems somewhat unstable
        const vector<shared_ptr<ISeekDownloader>> downloaders;
        
        std::pair<decltype(PlaybackData::downloaders)::const_iterator,int> findPart(int globalOffset) const;
    };
}

#endif /* defined(__G_Ear__PlaybackData__) */
