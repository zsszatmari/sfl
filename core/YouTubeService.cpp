//
//  YouTubeService.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/23/13.
//
//

#include "YouTubeService.h"
#include "YouTubeSession.h"
#include "IApp.h"

/*
// redirect uri: https://www.localhost.com/oauth2callback
static const std::string kClientId = "1007807607350-6p3ddpe2em5pa85ri5tjglftctln0p5m.apps.googleusercontent.com";
static const std::string kSecret = "gHL8RDpcFJHJZIgLkDpCuufT";
*/


namespace Gear
{
#define method YouTubeService::
    
    string method identifier()
    {
        return staticIdentifier();
    }

    string method staticIdentifier()
    {
        return "com.treasurebox.gear.account.youtube";
    }
    
    string method title() const
    {
        return "YouTube Personal";
    }
    
    shared_ptr<YouTubeService> method create(const shared_ptr<SessionManager> &sessionManager)
    {
        return shared_ptr<YouTubeService>(new YouTubeService(sessionManager));
    }
    
    method YouTubeService(const shared_ptr<SessionManager> &sessionManager) :
        DbService(sessionManager)
    {
    }

    string method inAppIdentifier() const
    {
        return "youtube";
    }
    
    bool method inAppNecessary() const
    {
        return true;
    }

    shared_ptr<DbSession> method createSession()
    {
        return YouTubeSession::create(IApp::instance());
    }
}
