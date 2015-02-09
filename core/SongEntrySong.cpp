#include "SongEntrySong.h"
#include "ISession.h"
#include "IApp.h"
#include "Db.h"
#include "SessionManager.h"

namespace Gear
{
#define method SongEntrySong::

	method SongEntrySong() :
        _values(Json::objectValue)
	{
	}

    method SongEntrySong(const shared_ptr<ISong> &rhs) :
        _values(MEMORY_NS::static_pointer_cast<const SongEntrySong>(rhs)->_values)
    {
    }

	void method setStringForKey(const string &key, const string &value)
	{
		_values[key] = value;
	}

    const string method uniqueId() const
    {
    	return _values.get("id","").asString();
    }
    
    string method stringForKey(const string &key) const
    {
        //Json::FastWriter writer;
        //std::cout << "song: " << writer.write(_values) << std::endl;
    	return _values.get(key,"").asString();
    }

    int64_t method intForKey(const string &key) const
    {
        Json::Value value = _values.get(key, 0);
        /*if (key == "lastPlayed" && value.asInt64() < 0) {
            std::cout << "lastPlayed\n";
        }*/
        if (value.isNumeric()) {
            return value.asInt64();
        }
    	return 0;
    }

    const float method floatForKey(const string &key) const
    {
        Json::Value value = _values.get(key, 0.0f);
        if (value.isNumeric()) {
            return value.asFloat();
        }
    	return 0.0f;
    }

    void method setIntForKey(const string &key, const int64_t &value)
    {
        // this is necessary for replica comparison. however, would be wrong for position!
        if (value == 0 && key == "replica") {
            _values.removeMember(key);
        } else {
        	_values[key] = value;
        }
    }

    void method setFloatForKey(const string &key, const float &value)
    {
    	_values[key] = value;
    }	

    void method save()
    {
        #pragma message("TODO: also save song modifications online")
        //std::cout << "db for songsave: " << IApp::instance()->db().get() << " app: " << IApp::instance().get() << std::endl;
        IApp::instance()->db()->upsert(shared_from_this());
    }

    shared_ptr<ISession> method session() const
    {
        return IApp::instance()->sessionManager()->session(stringForKey("source"));
    }
        
    // this method will block so it's best to call it synchronously
    shared_ptr<IPlaybackData> method playbackDataSync()
    {
        auto s = session();
        if (s) {
            return s->playbackDataSync(*this);
        } else {
            return shared_ptr<IPlaybackData>();
        }
    }
}