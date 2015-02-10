//
//  ModifiablePlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#ifndef __G_Ear_Player__StoredGooglePlaylist__
#define __G_Ear_Player__StoredGooglePlaylist__

#include "StoredPlaylist.h"
#include "SongManipulationSession.h"

namespace Gear
{
    class ModifiablePlaylist : public StoredPlaylist
    {
    public:
        static shared_ptr<ModifiablePlaylist> create(const string &playlistId, const string &name, const shared_ptr<SongManipulationSession> &session);

        virtual void addSongEntries(const vector<SongEntry> &songs);
        
        // these two are very dangerous: more than likely the songs are already added to the db
        //void addSongsLocally(const vector<SongEntry> &songs);
        //void setSongsLocally(const vector<SongEntry> &songs);
        virtual void setName(const string &name);
        virtual void remove();
        virtual void removeSongs(const vector<SongEntry> &songs);
        void setToken(const string &token);
        virtual bool moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before);
        const string &token() const;
        virtual bool removable() const;
        void setRemovable(bool value);
        
    protected:
        ModifiablePlaylist(const string &playlistId, const string &name, const shared_ptr<SongManipulationSession> &session);
        
    private:
        weak_ptr<SongManipulationSession> _session;
        string _token;
        
        bool _removable;
    };
}

#endif /* defined(__G_Ear_Player__StoredGooglePlaylist__) */
