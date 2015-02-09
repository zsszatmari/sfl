//
//  PlaybackItem.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/28/12.
//
//

#ifndef PLAYBACKITEM_H
#define PLAYBACKITEM_H

#include "stdplus.h"
#include "PlaybackData.h"
#include "SerialExecutor.h"
#include "CircularBuffer.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    class PlaybackItemDelegate;
    class IDecoder;
    
    class PlaybackItem final : public MEMORY_NS::enable_shared_from_this<PlaybackItem>
    {
    public:
        static shared_ptr<PlaybackItem> create(const shared_ptr<IPlaybackData> &data);
        void setDelegate(const shared_ptr<PlaybackItemDelegate> &delegate);
        int getUncompressedDataInto(char *buffer, int bufferLength);
        bool isSongFinished();
        float currentTime();
        void setCurrentTime(float desiredTime, float ratio);
        bool finishedDownloading();
        void setRateIfPlaying(float rate);
        
    private:
        PlaybackItem(const shared_ptr<IPlaybackData> &data);
        
        PlaybackItem(const PlaybackItem &rhs); // delete
        PlaybackItem &operator=(const PlaybackItem &rhs); // delete
        
        void processData();
        void waitForData();
        
        Base::SerialExecutor _executor;
        long _compressedDataPosition;
        bool _downloading;
        float _currentTime;
        shared_ptr<Gear::IDecoder> _decoder;
        Gear::CircularBuffer _uncompressedUnusedData;
        bool _initialBuffering;
        bool _finished;
        bool _reportFinished;
        const shared_ptr<IPlaybackData> _data;
        shared_ptr<PlaybackItemDelegate> _delegate;
        uint32_t _gapBytes;
        const uint32_t _gapBytesMax;
    };

#ifdef DEBUG
    void debugStreamAppend(const std::string &, const char *buf, int size);
#else
    #define debugStreamAppend(x,z,y)
#endif
}

#endif
