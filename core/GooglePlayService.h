//
//  GooglePlayService.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#ifndef __G_Ear_Player__GooglePlayService__
#define __G_Ear_Player__GooglePlayService__

#include "stdplus.h"
#include "IService.h"
#include SHAREDFROMTHIS_H
#include "DbService.h"

namespace Gear
{
    class SessionManager;
    class DbSession;
    
    class GooglePlayService final : public DbService
    {
    public:
        virtual string identifier();
        static string staticIdentifier();
        
        virtual string title() const;
        
        virtual string inAppIdentifier() const;
        virtual bool inAppNecessary() const;
        GooglePlayService(const shared_ptr<SessionManager> &serviceManager);
        
        virtual shared_ptr<DbSession> createSession();

    private:
    };
}

#endif /* defined(__G_Ear_Player__GooglePlayService__) */
