//
//  UnionPlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/27/13.
//
//

#ifndef __G_Ear_Player__UnionPlaylist__
#define __G_Ear_Player__UnionPlaylist__

#include "IPlaylist.h"

namespace Gear
{
    class UnionSongArray;
    class SortedSongArray;
    
    class UnionPlaylist final : public IPlaylist
    {
    public:
        UnionPlaylist(const string &playlistId, const string &name);
        
        virtual const string playlistId() const;
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const shared_ptr<ISongArray> songArray();
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        void setPlaylists(vector<shared_ptr<IPlaylist>> playlists);
        const shared_ptr<SortedSongArray> unionArray();
        virtual bool saveForOfflinePossible() const;
        virtual bool reorderable() const override;
        
        
    private:
        void recompute();
        
        vector<shared_ptr<IPlaylist>> _playlists;
        weak_ptr<UnionSongArray> _songArray;
        string _playlistId;
        string _name;
    };
}


#endif /* defined(__G_Ear_Player__UnionPlaylist__) */
