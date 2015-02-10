//
//  RemoteGooglePlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/20/13.
//
//

#include "RemoteGooglePlaylist.h"
#include "GooglePlaySession.h"
#include "GooglePlaySharedPlaylistArray.h"

namespace Gear
{
#define method RemoteGooglePlaylist::
    
    shared_ptr<RemoteGooglePlaylist> method create(const string &name, const string &playlistId, const string &shareToken, const weak_ptr<Gear::GooglePlaySession> &session)
    {
        shared_ptr<RemoteGooglePlaylist> ret(new RemoteGooglePlaylist(name, playlistId, shareToken, session));
        return ret;
    }
    
    method RemoteGooglePlaylist(const string &name, const string &playlistId, const string &shareToken, const weak_ptr<GooglePlaySession> &session) :
        IPlaylist(session),
        _name(name),
        _session(session),
        _playlistId(playlistId),
        _shareToken(shareToken)
    {
    }
    
    const shared_ptr<ISongArray> method songArray()
    {
        // TBD: how long do we want to retain radio songs?
        /*if (_retainedArray) {
         return _retainedArray;
         }*/
        
        auto array = _songArray.lock();
        if (array) {
            return array;
        }
        
        auto session = _session.lock();
        if (!session) {
            return array;
        }
        
        array = GooglePlaySharedPlaylistArray::create(_shareToken, session);
        _songArray = array;
        //_retainedArray = array;
        return array;
    }
    
    bool method removable() const
    {
        return true;
    }
    
    const string method name() const
    {
        return _name;
    }
    
    void method setName(const string &name)
    {
        _name = name;
    }
    
    const string method playlistId() const
    {
        return _playlistId;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>();
    }
    
    void method remove()
    {
        auto session = _session.lock();
        if (session) {
            session->removePlaylist(shared_from_this());
        }
    }
    
    bool method saveForOfflinePossible() const
    {
        // shared by others
        return false;
    }
}
