/*
 * LocalService.cpp
 *
 *  Created on: Nov 6, 2013
 *      Author: zsszatmari
 */

#include "LocalService.h"
#include "IApp.h"
#include "LocalSession.h"

namespace Gear
{
#define method LocalService::

    shared_ptr<LocalService> method create(shared_ptr<SessionManager> &sessionManager)
    {
        if (available()) {
            return shared_ptr<LocalService>(new LocalService(sessionManager));
        } else {
            return shared_ptr<LocalService>();
        }
    }

    method LocalService(shared_ptr<SessionManager> &sessionManager) :
        TrivialService(sessionManager)
    {
    }

    string method title() const
    {
        return "Local Music";
    }

    string method preferencesKey() const
    {
    	return "LocalMusicDisabled";
    }
    
    bool method available()
    {
        return (bool)IApp::instance()->createLocalSession();
    }

    shared_ptr<ISession> method createSession()
    {
    	auto ret = IApp::instance()->createLocalSession();
        ret->init(ret);
        return ret;
    }
    
    string method inAppIdentifier() const
    {
        return "local";
    }
    
    bool method inAppNecessary() const
    {
        return false;
    }

} /* namespace Gear */
