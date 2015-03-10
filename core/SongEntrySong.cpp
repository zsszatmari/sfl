#include <iostream>
#include "SongEntrySong.h"
#include "ISession.h"
#include "IApp.h"
#include "Db.h"
#include "SessionManager.h"

namespace Gear
{
#define method SongEntrySong::

	method SongEntrySong()
	{
	}

    method SongEntrySong(const shared_ptr<ISong> &rhs) :
        _values(MEMORY_NS::static_pointer_cast<const SongEntrySong>(rhs)->_values)
    {
    }

	void method setStringForKey(const string &key, const string &value)
	{
		_values.setStringForKey(key, value);
	}

    const string method uniqueId() const
    {
    	return _values.stringForKey("id");
    }
    
    string method stringForKey(const string &key) const
    {
        //Json::FastWriter writer;
        //std::cout << "song: " << writer.write(_values) << std::endl;
    	return _values.stringForKey(key);
    }

    int64_t method intForKey(const string &key) const
    {
        return _values.intForKey(key);
        /*if (key == "lastPlayed" && value.asInt64() < 0) {
            std::cout << "lastPlayed\n";
        }*/
    }

    const float method floatForKey(const string &key) const
    {
        return _values.floatForKey(key);
    }

    void method setIntForKey(const string &key, const int64_t &value)
    {
        _values.setIntForKey(key, value);
#pragma message("TODO: should handle replica a bit differently? for queue")
        // this is necessary for replica comparison. however, would be wrong for position!
/*        if (value == 0 && key == "replica") {
            _values.removeMember(key);
        } else {
        	_values[key] = value;
        }*/
    }

    void method setFloatForKey(const string &key, const float &value)
    {
    	_values.setFloatForKey(key,value);
    }	

#if 0
    void method save()
    {
        //std::cout << "db for songsave: " << IApp::instance()->db().get() << " app: " << IApp::instance().get() << std::endl;
        
#ifdef DEBUG
        //Json::FastWriter writer;
        //std::cout << "saving song to db: " << writer.write(_values) << std::endl;
#endif
        IApp::instance()->db()->upsert("Song", _values);
    }
#endif

    void method updateInDb(const vector<string> &fields)
    {
#pragma message("TODO: also save song modifications online. but not here! it should be explicit upload() or something")
#pragma message("TODO: save song modifications for all entries! for example, when title or offline ratio is changed, update entries")

        auto pred = [&](const string &key){return ClientDb::Predicate(key, stringForKey(key));};
        auto predicate = ClientDb::Predicate(ClientDb::Predicate::Operator::And, {
            pred("source"),pred("playlist"),pred("id")
        });
        
        auto values = _values.toJson();
        Json::Value updateFields;
        for (auto &field : fields) {
            updateFields[field] = values[field];
        }

        IApp::instance()->db()->update("Song", predicate, updateFields);        
    }

    shared_ptr<ISession> method session() const
    {
        return IApp::instance()->sessionManager()->session(stringForKey("source"));
    }
        
    // this method will block so it's best to call it synchronously
    /*shared_ptr<IPlaybackData> method playbackDataSync()
    {
        auto s = session();
        if (s) {
            return s->playbackDataSync(*this);
        } else {
            return shared_ptr<IPlaybackData>();
        }
    }*/
}