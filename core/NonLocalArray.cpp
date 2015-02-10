//
//  NonLocalArray.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/4/13.
//
//

#include "NonLocalArray.h"
#include "AllAccessToLibraryIntent.h"
#include "BaseUtility.h"

namespace Gear
{
#define method NonLocalArray::
    
    method NonLocalArray(const shared_ptr<ISession> &session, const ClientDb::Predicate &predicate) :
        SessionSongArray(session, predicate) 
    {
    }
    
    vector<string> method forbiddenColumns() const
    {
        string forbidden[] = {"genre", "currentInstant", "playCount", "lastPlayed", "creationDate"};
        return init<vector<string>>(forbidden);
    }
    
    vector<shared_ptr<ISongIntent>> method songIntents(const vector<SongEntry> &songs, const string &fieldHint) const
    {
        auto lSession = session().lock();
        if (lSession) {
            auto ret = SessionSongArray::songIntents(songs, fieldHint);
        	if (lSession->manipulationEnabled()) {
        		shared_ptr<ISongIntent> addLibrary(new AllAccessToLibraryIntent(lSession));
				ret.insert(ret.begin(), addLibrary);
        	}
            return ret;
        } else {
            return vector<shared_ptr<ISongIntent>>();
        }
    }

    const bool method orderedArray() const
	{
    	return true;
	}
}
