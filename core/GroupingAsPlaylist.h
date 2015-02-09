//
//  GroupingAsPlaylist.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 9/25/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__GroupingAsPlaylist__
#define __G_Ear_core__GroupingAsPlaylist__

#include "IPlaylist.h"

namespace Gear
{
    class GroupedPlaylist;
    class SortedSongArray;
    
    class GroupingAsPlaylist : public IPlaylist
    {
    public:
        static shared_ptr<GroupingAsPlaylist> create(const shared_ptr<IPlaylist> &playlist, const SongGrouping &grouping);
        
        virtual const string playlistId() const;
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const shared_ptr<ISongArray> songArray();
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        virtual shared_ptr<PromisedImage> image(int preferredSize) const;
        //virtual shared_ptr<ISession> session() const;
        
        virtual ~GroupingAsPlaylist();
        virtual bool saveForOfflinePossible() const;
        
        virtual std::pair<PlaylistCategory,shared_ptr<IPlaylist>> selected();
        virtual shared_ptr<const ISongArray> intentApplyArray() const;
        
    private:
        GroupingAsPlaylist(const shared_ptr<IPlaylist> &playlist, const SongGrouping &grouping);
        GroupingAsPlaylist(); // delete
        GroupingAsPlaylist(const GroupingAsPlaylist &rhs); // delete
        
        const shared_ptr<IPlaylist> _playlist;
        const SongGrouping _grouping;
        shared_ptr<SortedSongArray> _songArray;
        mutable shared_ptr<const ISongArray> _intentApplyArray;
    };
}

#endif /* defined(__G_Ear_core__GroupingAsPlaylist__) */
