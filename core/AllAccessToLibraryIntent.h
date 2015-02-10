//
//  AllAccessToLibraryIntent.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#ifndef __G_Ear__AllAccessToLibraryIntent__
#define __G_Ear__AllAccessToLibraryIntent__

#include "ISongNoIndexIntent.h"
#include "GooglePlaySession.h"

namespace Gear
{
    class AllAccessToLibraryIntent final : public ISongNoIndexIntent
    {
    public:
        AllAccessToLibraryIntent(const shared_ptr<ISession> &session);
        virtual const string menuText() const;
        virtual void apply(const vector<shared_ptr<ISong>> &songs) const;
        
    private:
        const weak_ptr<ISession> _session;
    };
}

#endif /* defined(__G_Ear__AllAccessToLibraryIntent__) */
