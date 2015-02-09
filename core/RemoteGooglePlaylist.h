//
//  RemoteGooglePlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/20/13.
//
//

#ifndef __G_Ear_Player__RemoteGooglePlaylist__
#define __G_Ear_Player__RemoteGooglePlaylist__

#include "IPlaylist.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    class GooglePlaySession;
    
    class RemoteGooglePlaylist : public IPlaylist, public MEMORY_NS::enable_shared_from_this<RemoteGooglePlaylist>
    {
    public:
        static shared_ptr<RemoteGooglePlaylist> create(const string &name, const string &playlistId, const string &shareToken, const weak_ptr<Gear::GooglePlaySession> &session);
        
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const shared_ptr<ISongArray> songArray();
        virtual bool removable() const;
        virtual void remove();
        virtual const string playlistId() const;
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        virtual bool saveForOfflinePossible() const;
    private:
        RemoteGooglePlaylist(const string &name, const string &playlistId, const string &shareToken, const weak_ptr<Gear::GooglePlaySession> &session);
        
        RemoteGooglePlaylist(const RemoteGooglePlaylist &rhs); // delete
        RemoteGooglePlaylist &operator=(const RemoteGooglePlaylist &rhs); // delete
        
        string _name;
        const string _shareToken;
        const string _playlistId;
        weak_ptr<ISongArray> _songArray;
        //shared_ptr<ISongArray> _retainedArray;
        const weak_ptr<Gear::GooglePlaySession> _session;
    };
}

#endif /* defined(__G_Ear_Player__RemoteGooglePlaylist__) */
