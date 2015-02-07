//
//  SpotifyData.cpp
//  G-Ear Player
//
//  Created by zsoltt on 10/15/13.
//
//

#include "SpotifyData.h"
#include "SpotifySession.h"

namespace Gear
{
#ifndef DISABLE_SPOTIFY

#define method SpotifyData::

    shared_ptr<SpotifyData> method create(const shared_ptr<SpotifySession> &session, sp_track *track)
    {
        return shared_ptr<SpotifyData>(new SpotifyData(session, track));
    }
    
    static long byteLengthFromTrack(sp_track *track)
    {
    	int millisecs = sp_track_duration(track);
		long totalLength = (millisecs/1000.0f) * 44100 * 2 * 2;
		return totalLength;
    }

    method SpotifyData(const shared_ptr<SpotifySession> &session, sp_track *track) :
        _session(session),
        _track(track),
        _buffer(65536),
        _offset(0)
    {
        sp_track_add_ref(track);
        // totalLength
        _totalLength = byteLengthFromTrack(track);
    }
    
    method ~SpotifyData()
    {
        sp_track_release(_track);
    }
    
    int method deliver(const sp_audioformat *format, const void *frames, int num_frames)
    {
        if (format->channels != 2) {
            return 0;
        }

        int availableBytes = 0;
        void *available = _buffer.head(availableBytes);
        if (!available) {
            return 0;
        }
        
        const int sampleSize = 2*2;
        int gotBytes = num_frames * sampleSize;
        if (gotBytes > availableBytes) {
            gotBytes = availableBytes;
        }
        
        memcpy(available, frames, gotBytes);
        _buffer.produce(gotBytes);
        
        function<void()> fToCall;
        {
            lock_guard<mutex> l(_asyncWaitMutex);
            if (_asyncWaitF) {
                if (offsetAvailable(_asyncWaitOffset) > 0) {
                    _asyncWaitF.swap(fToCall);
                }
            }
        }
        if (fToCall) {
            fToCall();
        }
        
        return gotBytes/sampleSize;
    }
    
    int method offsetAvailable(int requested) const
    {
        if (requested >= _offset) {
            int available = 0;
            _buffer.tail(available);
            available -= (requested - _offset);
            if (available > 0) {
                return available;
            }
        }
        return 0;
    }
    
    void method waitAsync(int offset, const function<void()> &f)
    {
        // trigger playback
        auto s = _session.lock();
        if (s) {
            s->startTrack(shared_from_this());
        }
        
        int available = offsetAvailable(offset);
        //std::cout << "waitasync.. offset needed: " << offset << " available: " << available << std::endl;
        if (available > 0) {
            f();
            return;
        }
        
        lock_guard<mutex> l(_asyncWaitMutex);
        _asyncWaitF = f;
        _asyncWaitOffset = offset;
    }
    
    void method access(int requested, const function<void(const char *ptr, int available)> &f)
    {
        if (requested < _offset) {
            // TODO problems...
        }
        if (requested > _offset) {
            _buffer.consume((int32_t)(requested - _offset));
            _offset = requested;
        }
        
        int available = 0;
        char *ptr = (char *)_buffer.tail(available);
        f(ptr, available);
    }
    
    void method seek(int offset)
    {
    	_offset = offset;
        float time = ((float)offset) / (44100 * 2 * 2);
        
        auto s = _session.lock();
        if (s) {
        	s->seekTrack(time);
        }
    }
    
    int method totalLength() const
    {
    	if (_totalLength == 0) {
    		// this is necessary because it might be that spotify haven't fetched our data yet
    		auto session = _session.lock();
    		if (session) {
    			session->executor().addTaskAndWait([&]{
    				_totalLength = byteLengthFromTrack(_track);
    			});
    		}
    	}
        return _totalLength;
    }
    
    bool method finished() const
    {
        return false;
    }
    
    int method failed() const
    {
        return 0;
    }
    
    IPlaybackData::Format method format() const
    {
        return IPlaybackData::Format::PcmSignedInt16;
    }

#endif
}
