//
//  PredicatePlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#include "PredicatePlaylist.h"
#include "IApp.h"
#include "PredicateSongArray.h"
#include "UnionSongArray.h"

namespace Gear
{
#define method PredicatePlaylist::

    using namespace Base;
    
    method PredicatePlaylist(const shared_ptr<SortedSongArray> &base, const string &name, const SongPredicate &predicate) :
        IPlaylist(weak_ptr<ISession>()),
        _name(name),
        _array(shared_ptr<ISongArray>(PredicateSongArray::create(base, predicate))),
        _playlistId(predicate.fingerPrint()),
        _saveForOfflinePossible(true)
    {
    }

    method PredicatePlaylist(const shared_ptr<SortedSongArray> &u, const string &name, const string &playlistId) :
        IPlaylist(weak_ptr<ISession>()),
        _name(name),
        _array(u),
        _playlistId(playlistId),
        _saveForOfflinePossible(true)
    {
    }
    
    shared_ptr<PredicatePlaylist> method thumbsUpPlaylist(const shared_ptr<SortedSongArray> &base)
    {
        return shared_ptr<PredicatePlaylist>(new PredicatePlaylist(base, u("Thumbs Up"), thumbsUpPredicate()));
    }
    
    SongPredicate method thumbsUpPredicate()
    {
        return SongPredicate("rating", 5);
    }

    SongPredicate method thumbsDownPredicate()
    {
        return SongPredicate("rating", 1);
    }

    shared_ptr<PredicatePlaylist> method highlyRatedPlaylist(const shared_ptr<SortedSongArray> &base, const shared_ptr<SortedSongArray> &thumbsUp)
    {
        #pragma message("TODO: highly rated playlist is a special case with different constructor")

        SongPredicate predicate("rating", 4, SongPredicate::GreaterOrEquals());

        auto u = UnionSongArray::create();
        auto fourArray = PredicateSongArray::create(base, SongPredicate("rating",4)); 
        vector<shared_ptr<SortedSongArray>> songArrays;
        songArrays.push_back(thumbsUp);
        songArrays.push_back(fourArray);
        u->setSongArrays(songArrays);
        return shared_ptr<PredicatePlaylist>(new PredicatePlaylist(u, "Highly Rated", predicate.fingerPrint()));
    }
    
    shared_ptr<PredicatePlaylist> method thumbsDownPlaylist(const shared_ptr<SortedSongArray> &base)
    {
        return shared_ptr<PredicatePlaylist>(new PredicatePlaylist(base, "Thumbs Down", thumbsDownPredicate()));
    }
    
     shared_ptr<PredicatePlaylist> method recentlyAddedPlaylist(const shared_ptr<SortedSongArray> &base)
    {
        return shared_ptr<PredicatePlaylist>(new PredicatePlaylist(base, "Recently Added", SongPredicate::recentlyAdded(IApp::instance()->preferences())));
    }
    
    shared_ptr<PredicatePlaylist> method unratedPlaylist(const shared_ptr<SortedSongArray> &base)
    {
        return shared_ptr<PredicatePlaylist>(new PredicatePlaylist(base, "Unrated", SongPredicate("rating", 0)));
    }
    
    const string method playlistId() const
    {
        return _playlistId;
    }
    
    void method setPlaylistId(const string &str)
    {
        _playlistId = str;
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
        return _array;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>();
    }
    
    bool method saveForOfflinePossible() const
    {
        return _saveForOfflinePossible;
    }
    
    void method setSaveForOfflinePossible(bool value)
    {
        _saveForOfflinePossible = value;
    }
}
