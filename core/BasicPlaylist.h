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
        BasicPlaylist(const weak_ptr<ISession> &session, const string &name, const string &playlistId, const shared_ptr<ISongArray> &songArray, const shared_ptr<IPlaylist> previous = nullptr, const std::map<string,string> &traits = std::map<string,string>());
        
        virtual const string playlistId() const;
        virtual const string name() const;
        virtual void setName(const string &name);
        virtual const shared_ptr<ISongArray> songArray();
        virtual shared_ptr<ISongNoIndexIntent> dragIntentTo(const vector<SongEntry> &songs);
        
        virtual bool saveForOfflinePossible() const;
        virtual void setFilterPredicate(const SongPredicate &f);
        virtual void refresh() override;
        
    private:
        const shared_ptr<ISongArray> _songArray;
        const string _playlistId;
        const string _name;
        const std::map<string,string> _traits;
    };

    shared_ptr<IPlaylist> playlistFrom(const ClientDb::Predicate &predicate,const string &playlistId, const string &name, const shared_ptr<ISession> &session, const std::map<string,string> &traits = std::map<string,string>());
    shared_ptr<IPlaylist> playlistFromData(const Json::Value &json);
}

#endif /* defined(__G_Ear_Player__BasicPlaylist__) */
