//
//  GooglePlayThumbsPlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/7/13.
//
//

#include "GooglePlayThumbsPlaylist.h"
#include "GooglePlayThumbsArray.h"

namespace Gear
{
#define method GooglePlayThumbsPlaylist::
    
    method GooglePlayThumbsPlaylist(const shared_ptr<GooglePlaySession> &session) :
    	IPlaylist(session),
        _session(session)
    {
    }
    
    void method reset()
    {
        _songArray.reset();
    }
    
    const shared_ptr<ISongArray> method songArray()
    {        
        shared_ptr<ISongArray> array = _songArray.lock();
        if (array) {
            return array;
        }
        
        auto session = _session.lock();
        if (!session) {
            return array;
        }
        
        array = GooglePlayThumbsArray::create(session);
        _songArray = array;
        
        //_retainedArray = array;
        return array;
    }

    const string method name() const
    {
        return u("Thumbs Up");
    }
    
    void method setName(const string &name)
    {
    }
    
    const string method playlistId() const
    {
        return "thumbsup";
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>();
    }

    bool method saveForOfflinePossible() const
    {
        return true;
    }
}
