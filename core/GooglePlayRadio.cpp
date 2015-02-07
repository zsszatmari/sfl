//
//  GooglePlayRadio.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/4/13.
//
//

#include "GooglePlayRadio.h"
#include "GooglePlayRadioArray.h"
#include "GooglePlaySession.h"
#include "PlaylistDeleteIntent.h"

namespace Gear
{
#define method GooglePlayRadio::
    
    shared_ptr<GooglePlayRadio> method create(const string &name, const string &radioId, const weak_ptr<GooglePlaySession> &session)
    {
        return shared_ptr<GooglePlayRadio>(new GooglePlayRadio(name, radioId, session));
    }
    
    method GooglePlayRadio(const string &name, const string &radioId, const weak_ptr<GooglePlaySession> &session) :
    	IPlaylist(session),
        _name(name),
        _radioSession(session),
        _radioId(radioId),
        _editable(true)
    {
    }

    void method refresh()
    {
        auto s = _songArray.lock();
        if (s) {
#pragma message("TODO: make radio refresh possible again")
            //s->removeSongsOtherThan(vector<SongEntry>());
            //s->songsLeft(0);
        }
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
        
        auto session = _radioSession.lock();
        if (!session) {
            return array;
        }
        
        array = GooglePlayRadioArray::create(_radioId, session);
        _songArray = array;
        //_retainedArray = array;
        return array;
    }
    
    bool method removable() const
    {
        // empty means i'm feeling lucky radio: cannot be removed
        return !_radioId.empty();
    }
    
    void method remove()
    {
        auto session = _radioSession.lock();
        if (session) {
            session->removeRadio(*this);
        }
    }
    
    const string method name() const
    {
        return _name;
    }
    
    void method setName(const string &name)
    {
        _name = name;
    }

    
    const string &method radioId() const
    {
        return _radioId;
    }
    
    const string method playlistId() const
    {
        return _radioId;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>();
    }

    shared_ptr<PromisedImage> method image(int preferredSize) const
	{
		return emptyImage();
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
        return false;
    }

    bool method keepSongOrder() const
    {
        return false;
    }

    bool method editable() const
    {
        return _editable;
    }

    void method setEditable(bool editable)
    {
        _editable = editable;
    }
}
