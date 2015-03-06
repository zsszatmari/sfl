//
//  IPlaylist.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/4/13.
//
//

#ifndef __G_Ear__IPlaylist__
#define __G_Ear__IPlaylist__

#include <vector>
#include <string>
#include "stdplus.h"
#include "ValueSignal.h"
#include "ManagedValue.h"
#include "SongGrouping.h"
#include "SongEntry.h"
#include "SongPredicate.h"
#include "PlaylistCategory.h"

namespace Gear
{
    using std::vector;
    using std::string;
    
    class ISongArray;
    class ISongNoIndexIntent;
    class ISong;
    class IPlaylistIntent;
    class SortDescriptor;
    
    class core_export IPlaylist
    {
    public:
        IPlaylist(const weak_ptr<ISession> &session);
        virtual ~IPlaylist();
        bool operator==(const IPlaylist &rhs) const;
        virtual const string playlistId() const = 0;
        virtual const string name() const = 0;
        virtual void setName(const string &name) = 0;
        virtual const shared_ptr<ISongArray> songArray() = 0;
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs) = 0;
        virtual bool removable() const;
        virtual void remove();
        virtual bool editable() const;
        virtual bool nameEditable() const;
        virtual bool reorderable() const;
        virtual void addSongEntries(const vector<SongEntry> &songs);
        virtual void removeSongs(const vector<SongEntry> &songs);
        virtual bool moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before);
        virtual bool orderedPlaylist() const;
        void setOrdered(bool ordered);
        virtual bool groupingsEnabled() const;
        virtual vector<shared_ptr<IPlaylistIntent>> playlistIntents();
        static vector<shared_ptr<IPlaylistIntent>> playlistIntents(const vector<shared_ptr<IPlaylist>> &);
        
        virtual shared_ptr<PromisedImage> image(int preferredSize) const;
        virtual shared_ptr<ISession> session() const;
        
        Base::ValueConnector<vector<SongGrouping>> availableGroupingsConnector();
        // pass true as second argument if and only if double clicked
        virtual void selectGrouping(const SongGrouping &grouping, bool play);
        Base::ValueConnector<SongGrouping> selectedGroupingConnector();
        virtual void setFilterPredicate(const SongPredicate &f);
        const SongPredicate & filterPredicate() const;
        virtual bool saveForOfflinePossible() const = 0;
        static bool saveForOfflineAsPlaylist();
        virtual std::pair<PlaylistCategory,shared_ptr<IPlaylist>> selected();
        virtual shared_ptr<const ISongArray> intentApplyArray() const;
        virtual bool keepSongOrder() const;
        void setSortDescriptor(const SortDescriptor &rhs);
        static SortDescriptor sortDescriptorForId(const string &playlistId);
        static SortDescriptor sortDescriptorGlobal();
        std::pair<string,bool> sortKeyAndAscending() const;
        virtual void refresh();
        shared_ptr<IPlaylist> previous() const;

        //virtual bool saveForOfflineAsPlaylist() const;
    
    protected:
        static shared_ptr<PromisedImage> emptyImage();

        Base::ManagedValue<vector<SongGrouping>> _availableGroupings;
        Base::ManagedValue<SongGrouping> _selectedGrouping;
        
        SongPredicate _filterPredicate;
        const weak_ptr<ISession> _session;
        shared_ptr<IPlaylist> _previous;

    private:
        const shared_ptr<const ISongArray> songArray() const;
        
        bool _ordered;
    };
};

#endif /* defined(__G_Ear__IPlaylist__) */
