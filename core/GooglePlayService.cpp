//
//  GooglePlayService.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#include "GooglePlayService.h"
#include "GooglePlaySession.h"
#include "IApp.h"

namespace Gear
{
#define method GooglePlayService::
    
    method GooglePlayService(const shared_ptr<SessionManager> &serviceManager) :
        DbService(serviceManager)
    {
    }

    string method staticIdentifier()
    {
        return "com.treasurebox.gear.account.googlemusic-cookies";
    }

    string method identifier()
    {
        return staticIdentifier();
    }
    
    string method title() const 
    {
        return "Google Music";
    }
    
    string method inAppIdentifier() const
    {
        return "googlemusic";
    }
    
    bool method inAppNecessary() const
    {
        return true;
    }

    shared_ptr<DbSession> method createSession()
    {
        return GooglePlaySession::create(IApp::instance());
    }

}
