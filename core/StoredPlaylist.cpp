//
//  StoredPlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#include "StoredPlaylist.h"
#include "AddToPlaylistIntent.h"
#include "PlaylistDeleteIntent.h"

namespace Gear
{
#define method StoredPlaylist::
    
    /*method StoredPlaylist(const shared_ptr<ManagedObject> &underlyingObject, const shared_ptr<ISession> &session) :
        IPlaylist(session),
        _session(session),
        _underlyingObject(underlyingObject),
        _editable(true),
        _saveForOfflinePossible(true)
    {
    }
    
    shared_ptr<ManagedObject> method headlessObject(const string &uniqueId)
    {
        shared_ptr<ManagedObject> ret(new ManagedObject(ISession::playlistEntity()));
        //cout << "bluu underlying:" << (long long)ret.get() << endl;
        ret->setValueForKey("id", uniqueId);
        return ret;
    }*/
    
    method StoredPlaylist(const string &playlistId, const string &name, const shared_ptr<ISession> &session) :
        IPlaylist(session),
        _playlistId(playlistId),
        _session(session),
        _name(name)
    {
        // TODO: use StoredSongArray
    }
        

    method ~StoredPlaylist()
    {
        // do nothing, essentially this is a debug hook
    }
    
    void method init()
    {
        _storedArray = shared_ptr<StoredSongArray>(new StoredSongArray(_session,shared_from_this()));
        
        // this is necessary so that we don't hold onto the session anymore
        _session.reset();
    }
    
    const shared_ptr<StoredSongArray> method storedSongArray()
    {
        return _storedArray;
    }
    
    const shared_ptr<ISongArray> method songArray()
    {
        return _storedArray;
    }
    
    const string method name() const
    {
        return _name;
    }
    
    void method setName(const string &name)
    {
        setNameLocally(name);
    
        // must override to update name in the repo
    }
    
    void method setNameLocally(const string &name)
    {
        #pragma message("TODO: rename playlist (locally)")
        //_underlyingObject->setValueForKey("name", name);
    }
    
    bool method editable() const
    {
        return _editable;
    }
    
    const string method playlistId() const
    {
        #pragma message("TODO: get playlistId")
        return _playlistId;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>(new AddToPlaylistIntent(shared_from_this()));
    }
    
    bool method removable() const
    {
        return true;
    }
    
    vector<shared_ptr<IPlaylistIntent>> method playlistIntents()
    {
        vector<shared_ptr<IPlaylistIntent>> ret = IPlaylist::playlistIntents();
        if (removable()) {
            ret.push_back(shared_ptr<PlaylistDeleteIntent>(new PlaylistDeleteIntent(shared_from_this())));
        }
        return ret;
    }
    
    bool method saveForOfflinePossible() const
    {
        return _saveForOfflinePossible;
    }
    
    void method setEditable(bool editable)
    {
        _editable = editable;
    }
    
    void method setSaveForOfflinePossible(bool saveForOfflinePossible)
    {
        _saveForOfflinePossible = saveForOfflinePossible;
    }
}
