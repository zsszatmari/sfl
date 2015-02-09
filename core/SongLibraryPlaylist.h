//
//  LibraryPlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/16/13.
//
//

#ifndef __G_Ear_Player__LibraryPlaylist__
#define __G_Ear_Player__LibraryPlaylist__

#include "StoredSongArray.h"
#include "StoredPlaylist.h"

namespace Gear
{
    class SongLibraryPlaylist : public IPlaylist, public MEMORY_NS::enable_shared_from_this<SongLibraryPlaylist>
    {
    public:
        static shared_ptr<SongLibraryPlaylist> create(const shared_ptr<ISession> &session);
        
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        
        virtual const shared_ptr<ISongArray> songArray();
        virtual void init();
        const shared_ptr<StoredSongArray> storedSongArray();
        virtual const string playlistId() const;
        void setPlaylistId(const string &playlistId);
        virtual const string name() const;
        virtual void setName(const string &name);
        void setOrdered(bool ordered);
        virtual void removeSongs(const vector<SongEntry> &songs);
        virtual bool orderedPlaylist() const;
        void addSongsLocally(const vector<SongEntry> &songs);
        // dangerous, should only keep ids lingering around
        //void removeSongsOtherThanLocally(const vector<SongEntry> &songs);
        virtual shared_ptr<ISession> session();
        
        virtual bool saveForOfflinePossible() const;
        
    protected:
        SongLibraryPlaylist(const shared_ptr<ISession> &session);
        
    private:
        shared_ptr<StoredSongArray> _songArray;
        string _name;
        string _playlistId;
        bool _ordered;
    };
}

#endif /* defined(__G_Ear_Player__LibraryPlaylist__) */
