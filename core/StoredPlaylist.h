//
//  StoredPlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#ifndef __G_Ear_Player__StoredPlaylist__
#define __G_Ear_Player__StoredPlaylist__

#include "IPlaylist.h"
#include "StoredSongArray.h"
    
namespace Gear
{
    class StoredPlaylist : public IPlaylist, public MEMORY_NS::enable_shared_from_this<StoredPlaylist>
    {
    public:
        // see ModifiablePlaylist instead, but StoredPlaylist will do the heavier lifting
        //static shared_ptr<StoredPlaylist> create(const string &source, const string &playlistId, const string &name);

        virtual const shared_ptr<ISongArray> songArray();
        const shared_ptr<StoredSongArray> storedSongArray();
        virtual const string name() const;
        virtual void setName(const string &name);
        void setNameLocally(const string &name);
        virtual bool editable() const;
        
        virtual void addSongEntries(const vector<SongEntry> &songs) = 0;
        virtual void remove() = 0;
        virtual const string playlistId() const;
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        virtual ~StoredPlaylist();
        virtual bool removable() const;
        virtual vector<shared_ptr<IPlaylistIntent>> playlistIntents();
        
        void setEditable(bool editable);
        void setSaveForOfflinePossible(bool saveForOfflinePossible);
        virtual bool saveForOfflinePossible() const;

    protected:
        // source identifier comes from ISession
        StoredPlaylist(const string &playlistId, const string &name, const shared_ptr<ISession> &session);
        virtual void init();
        shared_ptr<StoredSongArray> _storedArray;
        
    private:
        const string _playlistId;
        const string _name;

        shared_ptr<ISession> _session;
        bool _editable;
        bool _saveForOfflinePossible;
    };
}

#endif /* defined(__G_Ear_Player__StoredPlaylist__) */
