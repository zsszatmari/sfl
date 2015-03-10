//
//  YouTubeSearchService.cpp
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/8/13.
//
//

#include "YouTubeSearchService.h"
#include "YouTubeSearchSession.h"
#include "IApp.h"

namespace Gear
{
#define method YouTubeSearchService::
    
    shared_ptr<YouTubeSearchService> method create(shared_ptr<SessionManager> &sessionManager)
    {
        return shared_ptr<YouTubeSearchService>(new YouTubeSearchService(sessionManager));
    }
    
    method YouTubeSearchService(shared_ptr<SessionManager> &sessionManager) :
        DbService(sessionManager)
    {
    }
    
    string method title() const
    {
        return "YouTube Search";
    }
    
    /*
    string method preferencesKey() const
    {
    	return "YouTubeSearchDisabled";
    }*/

    shared_ptr<DbSession> method createSession()
    {
    	return YouTubeSearchSession::create(IApp::instance());
    }

    string method identifier()
    {
        return "youtube-search";
    }
    
    string method inAppIdentifier() const
    {
        return "youtubesearch";
    }
    
    bool method inAppNecessary() const
    {
        return false;
    }
}
