//
//  YouTubeSearchService.h
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/8/13.
//
//

#ifndef __G_Ear_Player__YouTubeSearchService__
#define __G_Ear_Player__YouTubeSearchService__

#include "DbService.h"

namespace Gear
{
    class YouTubeSearchService final : public DbService
    {
    public:
        static shared_ptr<YouTubeSearchService> create(shared_ptr<SessionManager> &sessionManager);
        
        virtual string identifier();
        
        virtual string title() const;
        
        virtual string inAppIdentifier() const;
        virtual bool inAppNecessary() const;

        virtual shared_ptr<DbSession> createSession();


    private:
        YouTubeSearchService(shared_ptr<SessionManager> &sessionManager);
    };
}

#endif /* defined(__G_Ear_Player__YouTubeSearchService__) */
