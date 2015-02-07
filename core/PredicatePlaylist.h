//
//  PredicatePlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__PredicatePlaylist__
#define __G_Ear_Player__PredicatePlaylist__

#include "IPlaylist.h"

namespace Gear
{
    class SortedSongArray;
    class SongPredicate;
    
    class PredicatePlaylist : public IPlaylist
    {
    public:
        static shared_ptr<PredicatePlaylist> thumbsUpPlaylist(const shared_ptr<SortedSongArray> &base);
        static shared_ptr<PredicatePlaylist> thumbsDownPlaylist(const shared_ptr<SortedSongArray> &base);
        static shared_ptr<PredicatePlaylist> highlyRatedPlaylist(const shared_ptr<SortedSongArray> &base, const shared_ptr<SortedSongArray> &thumbsUp);
        static shared_ptr<PredicatePlaylist> recentlyAddedPlaylist(const shared_ptr<SortedSongArray> &base);
        static shared_ptr<PredicatePlaylist> unratedPlaylist(const shared_ptr<SortedSongArray> &base);
        static SongPredicate thumbsUpPredicate();
        static SongPredicate thumbsDownPredicate();
        
        PredicatePlaylist(const shared_ptr<SortedSongArray> &base, const string &name, const SongPredicate &predicate);
        
        virtual const string playlistId() const;
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const shared_ptr<ISongArray> songArray();
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        virtual bool saveForOfflinePossible() const;
        void setSaveForOfflinePossible(bool);
        
        void setPlaylistId(const string &str);
        
    private:
        PredicatePlaylist(const shared_ptr<SortedSongArray> &u, const string &name, const string &playlistId);

        shared_ptr<ISongArray> _array;
        string _name;
        string _playlistId;
        
        bool _saveForOfflinePossible;
    };
}

#endif /* defined(__G_Ear_Player__PredicatePlaylist__) */
