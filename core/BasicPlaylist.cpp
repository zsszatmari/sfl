//
//  BasicPlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/7/13.
//
//

#include "BasicPlaylist.h"

namespace Gear
{
#define method BasicPlaylist::
    
    method BasicPlaylist(const weak_ptr<ISession> &session, const string &name, const string &playlistId, const shared_ptr<ISongArray> &songArray) :
    	IPlaylist(session),
        _name(name),
        _playlistId(playlistId),
        _songArray(songArray)
    {
    }
    
    const string method playlistId() const
    {
        return _playlistId;
    }
    
    const string method name() const
    {
        return _name;
    }
    
    void method setName(const string &name)
    {
    }
    
    const shared_ptr<ISongArray> method songArray()
    {
        return _songArray;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>();
    }
    
    
    bool method saveForOfflinePossible() const
    {
        return false;
    }
}
