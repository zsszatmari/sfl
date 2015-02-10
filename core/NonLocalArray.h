//
//  NonLocalArray.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/4/13.
//
//

#ifndef __G_Ear_Player__NonLocalArray__
#define __G_Ear_Player__NonLocalArray__

#include "SessionSongArray.h"

namespace Gear
{
    class NonLocalArray : public SessionSongArray
    {
    public:
        NonLocalArray(const shared_ptr<ISession> &session, const ClientDb::Predicate &predicate);
        
        const bool orderedArray() const;

        virtual vector<string> forbiddenColumns() const;
        virtual vector<shared_ptr<ISongIntent>> songIntents(const vector<SongEntry> &songs, const string &fieldHint) const;
    };
}

#endif /* defined(__G_Ear_Player__NonLocalArray__) */
