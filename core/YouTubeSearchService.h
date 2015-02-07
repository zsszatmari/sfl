//
//  YouTubeSearchService.h
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/8/13.
//
//

#ifndef __G_Ear_Player__YouTubeSearchService__
#define __G_Ear_Player__YouTubeSearchService__

#include "TrivialService.h"

namespace Gear
{
    class YouTubeSearchService final : public TrivialService
    {
    public:
        static shared_ptr<YouTubeSearchService> create(shared_ptr<SessionManager> &sessionManager);
        virtual string title() const;
        
        virtual string inAppIdentifier() const;
        virtual bool inAppNecessary() const;

    private:
        virtual shared_ptr<ISession> createSession();
        virtual string preferencesKey() const;

        YouTubeSearchService(shared_ptr<SessionManager> &sessionManager);
    };
}

#endif /* defined(__G_Ear_Player__YouTubeSearchService__) */
