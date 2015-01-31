//
//  PlaylistSession.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/14/13.
//
//

#ifndef __G_Ear_Player__PlaylistSession__
#define __G_Ear_Player__PlaylistSession__

#include <functional>
#include "ISession.h"
#include "PlaylistCategory.h"
#include "AtomicPtr.h"

namespace Gear
{
    using std::function;

    class PlaylistSession : public ISession
    {
    public:
        PlaylistSession(const shared_ptr<IApp> &app);
        
        static const function<void(const shared_ptr<IPlaylist> &)> discard;
        
        void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const function<void(const shared_ptr<IPlaylist> &)> &result = discard);
        void createUserPlaylist(const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result = discard);
        virtual vector<shared_ptr<IPlaylist>> playlists() = 0;

    protected:
        void modifyCategoryByTag(int tag, const function<void(PlaylistCategory &)> &modify);
        //void modifyCategoryByTag(int tag, const function<PlaylistCategory(const PlaylistCategory &)> &modify);

        PlaylistCategory readCategoryByTag(int tag) const;
        
        AtomicPtr<vector<PlaylistCategory>> _categories;
         
    private:
        virtual void createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result) = 0;
    };
}

#endif /* defined(__G_Ear_Player__PlaylistSession__) */
