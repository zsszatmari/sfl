#ifndef FAKEPLAYBACK_DATA_H 
#define FAKEPLAYBACK_DATA_H 

#include "IPlaybackData.h"

using namespace Gear;
using std::function;

class FakePlaybackData : public IPlaybackData
{
public:
	FakePlaybackData() : IPlaybackData(Format::Mp3) {}
	virtual void seek(int offset) {}
	virtual int offsetAvailable(int offset) const {return 0;}

    virtual void accessChunk(int offset, const function<void(const char *ptr, int available)> &f) {}
        
    virtual int totalLength() const {return 0;}
    virtual bool finished() const {return false;}
    virtual int failed() const {return false;}
};

#endif