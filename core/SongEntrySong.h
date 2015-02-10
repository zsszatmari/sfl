#ifndef SONGENTRYSONG_H
#define SONGENTRYSONG_H

#include "json.h"
#include "stdplus.h"
#include "ISong.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    using std::string;

	class SongEntrySong final : public ISong, public MEMORY_NS::enable_shared_from_this<SongEntrySong>
    {
    public:
    	SongEntrySong();
        SongEntrySong(const shared_ptr<ISong> &rhs);
    	virtual void setStringForKey(const string &key, const string &value);
        virtual const string uniqueId() const;
        virtual string stringForKey(const string &key) const;
        virtual int64_t intForKey(const string &key) const;
        virtual const float floatForKey(const string &key) const;
        virtual void setIntForKey(const string &key, const int64_t &value);
        virtual void setFloatForKey(const string &key, const float &value);
        virtual void save();
        virtual shared_ptr<ISession> session() const;
        
        // this method will block so it's best to call it synchronously
        virtual shared_ptr<IPlaybackData> playbackDataSync();

    private:
        Json::Value _values;

        friend class Db;
        friend class SongEntry;
    };

}

#endif