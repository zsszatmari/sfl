//
//  SessionSongArray.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/5/13.
//
//

#ifndef __G_Ear__SessionSongArray__
#define __G_Ear__SessionSongArray__

#include "SortedSongArray.h"
#include "ISession.h"
#include "IApp.h"

namespace Gear
{
    class ISongIntent;
    
    class SessionSongArray : public SortedSongArray
    {
    public:
        SessionSongArray(const shared_ptr<ISession> &session, const ClientDb::Predicate &predicate);
        
    protected:
        const weak_ptr<ISession> session() const;
        
    private:
        const weak_ptr<ISession> _session;
        const weak_ptr<IApp> _app;
    };
}

#endif /* defined(__G_Ear__SessionSongArray__) */
