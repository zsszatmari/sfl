//
//  GroupedPlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/1/13.
//
//

#ifndef __G_Ear_Player__GroupedPlaylist__
#define __G_Ear_Player__GroupedPlaylist__

#include "IPlaylist.h"
#include "SongPredicate.h"
#include SHAREDFROMTHIS_H


namespace Gear
{
    class SortedSongArray;
    
    class GroupedPlaylist final : public IPlaylist, public MEMORY_NS::enable_shared_from_this<GroupedPlaylist>
    {
    public:
        /*static shared_ptr<GroupedPlaylist> albumsPlaylist(const shared_ptr<SortedSongArray> &base);
        static shared_ptr<GroupedPlaylist> artistsPlaylist(const shared_ptr<SortedSongArray> &base);
        static shared_ptr<GroupedPlaylist> genresPlaylist(const shared_ptr<SortedSongArray> &base);
        static shared_ptr<GroupedPlaylist> albumArtistsPlaylist(const shared_ptr<SortedSongArray> &base);
        */

        virtual const string playlistId() const;
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const shared_ptr<ISongArray> songArray();
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        virtual bool groupingsEnabled() const;
        virtual void selectGrouping(const SongGrouping &grouping, bool play);
        
        virtual void setFilterPredicate(const SongPredicate &predicate);
        
        virtual bool saveForOfflinePossible() const;
        
        shared_ptr<ISongArray> unfilteredArray() const;
        
        vector<SongGrouping> groupSongs(const ClientDb::Predicate &predicate, const vector<string> &groupBy);
        
        static shared_ptr<GroupedPlaylist> create(const shared_ptr<SortedSongArray> &base, const string &name, const vector<string> &groupBy, const shared_ptr<IPlaylist> &previous);

    private:
        GroupedPlaylist(const shared_ptr<SortedSongArray> &base, const string &name, const vector<string> &groupBy, const shared_ptr<IPlaylist> &previous);
        
        void recalculate();
        
        string _name;
        string _playlistId;
        shared_ptr<SortedSongArray> _array;
        vector<string> _groupBy;
        SongPredicate _predicate;
        int _calculateId;

        Base::SignalConnection _resortConnection;
    };
}

#endif /* defined(__G_Ear_Player__GroupedPlaylist__) */
