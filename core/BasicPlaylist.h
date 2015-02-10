//
//  BasicPlaylist.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/7/13.
//
//

#ifndef __G_Ear_Player__BasicPlaylist__
#define __G_Ear_Player__BasicPlaylist__

#include "IPlaylist.h"

namespace Gear
{
    class BasicPlaylist : public IPlaylist
    {
    public:
        BasicPlaylist(const weak_ptr<ISession> &session, const string &name, const string &playlistId, const shared_ptr<ISongArray> &songArray);
        
        virtual const string playlistId() const;
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const shared_ptr<ISongArray> songArray();
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        
        virtual bool saveForOfflinePossible() const;
        
    private:
        const shared_ptr<ISongArray> _songArray;
        const string _playlistId;
        const string _name;
    };
}

#endif /* defined(__G_Ear_Player__BasicPlaylist__) */
