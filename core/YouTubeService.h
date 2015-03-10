//
//  GooglePlayService.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#ifndef __G_Ear_Player__YouTubeService__
#define __G_Ear_Player__YouTubeService__

#include "stdplus.h"
#include "IService.h"
#include SHAREDFROMTHIS_H
#include "DbService.h"

namespace Gear
{
    class SessionManager;
    class DbSession;
    
    class YouTubeService final : public DbService
    {
    public:
        static string staticIdentifier();
        virtual string identifier();
        
        virtual string title() const;
        
        virtual string inAppIdentifier() const;
        virtual bool inAppNecessary() const;
        static shared_ptr<YouTubeService> create(const shared_ptr<SessionManager> &sessionManager);
        
        shared_ptr<DbSession> createSession();

    private:
        YouTubeService(const shared_ptr<SessionManager> &serviceManager);
    };
}

#endif /* defined(__G_Ear_Player__GooglePlayService__) */
