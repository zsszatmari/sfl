//
//  PredicateSongArray.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__PredicateSongArray__
#define __G_Ear_Player__PredicateSongArray__

#include "SortedSongArray.h"
#include "SignalConnection.h"
#include "Timer.h"
#include SHAREDFROMTHIS_H

namespace Gear
{    
    class PredicateSongArray final : public SortedSongArray, public MEMORY_NS::enable_shared_from_this<PredicateSongArray>
    {
    public:
        static shared_ptr<PredicateSongArray> create(const shared_ptr<SortedSongArray> &base, const SongPredicate &predicate);
        virtual vector<shared_ptr<ISongIntent>> songIntents(const vector<SongEntry> &songs, const string &fieldHint) const;
		virtual vector<pair<string, vector<shared_ptr<ISongIntent>>>> songIntentSubMenus(const vector<SongEntry> &songs) const;
		virtual void resetCache();

    private:
        PredicateSongArray(const shared_ptr<SortedSongArray> &base, const SongPredicate &predicate);
        
        void initUpdate(const shared_ptr<SortedSongArray> &base);
        
        Base::Timer _rateTimer; 
        Base::SignalConnection _updateConnection;
        Base::SignalConnection _ratedConnection;
        shared_ptr<ISongArray> _base;
    };
}

#endif /* defined(__G_Ear_Player__PredicateSongArray__) */
