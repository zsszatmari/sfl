//
//  RemoveFromLibraryIntent.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/17/13.
//
//

#ifndef __G_Ear_Player__RemoveFromLibraryIntent__
#define __G_Ear_Player__RemoveFromLibraryIntent__

#include "ISongIntent.h"
#include "GearUtility.h"
#include WEAK_H
using MEMORY_NS::weak_ptr;

namespace Gear
{
	class ISession;

    class RemoveFromLibraryIntent final : public ISongIntent
    {
    public:
        RemoveFromLibraryIntent(const shared_ptr<ISession> &session);
        
        virtual const string menuText() const;
        virtual void apply(const vector<SongEntry> &songs) const;
        virtual bool confirmationNeeded(const vector<SongEntry> &songs) const;
        virtual string confirmationText(const vector<SongEntry> &songs) const;
        
    private:
        const weak_ptr<ISession> _session;
    };
}

#endif /* defined(__G_Ear_Player__RemoveFromLibraryIntent__) */
