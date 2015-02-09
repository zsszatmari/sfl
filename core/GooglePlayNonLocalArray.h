//
//  GooglePlayNonLocalArray.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/12/13.
//
//

#ifndef __G_Ear_Player__GooglePlayNonLocalArray__
#define __G_Ear_Player__GooglePlayNonLocalArray__

#include "NonLocalArray.h"
#include "GooglePlaySession.h"

namespace Gear
{
    class GooglePlayNonLocalArray : public NonLocalArray
    {
    public:
        GooglePlayNonLocalArray(const shared_ptr<ISession> &session, const ClientDb::Predicate &predicate);
        
    protected:
        const weak_ptr<GooglePlaySession> session() const;
    };
}

#endif /* defined(__G_Ear_Player__GooglePlayNonLocalArray__) */
