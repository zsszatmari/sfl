//
//  SpotifyData.h
//  G-Ear Player
//
//  Created by zsoltt on 10/15/13.
//
//

#ifndef __G_Ear_Player__SpotifyData__
#define __G_Ear_Player__SpotifyData__

#include "SpotifySession.h"
#include "IPlaybackData.h"
#include "CircularBuffer.h"

namespace Gear
{
#ifndef DISABLE_SPOTIFY

    class SpotifyData final : public IPlaybackData, public MEMORY_NS::enable_shared_from_this<SpotifyData>
    {
    public:
        static shared_ptr<SpotifyData> create(const shared_ptr<SpotifySession> &session, sp_track *track);
        ~SpotifyData();
        
        virtual void waitAsync(int offset, const function<void()> &f);
        
        virtual int offsetAvailable(int offset) const;
        
        virtual void access(int offset, const function<void(const char *ptr, int available)> &f);
        virtual void seek(int offset);
        
        virtual int totalLength() const;
        virtual bool finished() const;
        virtual int failed() const;
        
        virtual Format format() const;
        
        int deliver(const sp_audioformat *format, const void *frames, int num_frames);
        
    private:
        SpotifyData(const shared_ptr<SpotifySession> &session, sp_track *track);
        
        weak_ptr<SpotifySession> _session;
        sp_track *_track;
        long _offset;
        mutable long _totalLength;
        mutable CircularBuffer _buffer;
        
        mutex _asyncWaitMutex;
        int _asyncWaitOffset;
        function<void()> _asyncWaitF;
        
        friend class SpotifySession;
    };

#endif
}

#endif /* defined(__G_Ear_Player__SpotifyData__) */
