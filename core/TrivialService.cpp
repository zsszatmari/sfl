/*
 * TrivialService.cpp
 *
 *  Created on: Nov 6, 2013
 *      Author: zsszatmari
 */

#include "TrivialService.h"
#include "SessionManager.h"

namespace Gear
{
#define method TrivialService::

	method TrivialService(shared_ptr<SessionManager> &sessionManager) :
        _sessionManager(sessionManager)
    {
    }

    void method connected()
    {
        _sessionManager->removeSession(_activeSession.lock());

        auto session = createSession();
        _activeSession = session;
        _sessionManager->addSession(session);

        session->refresh();
    }
    
    void method disconnected()
    {
        _sessionManager->removeSession(_activeSession.lock());
    }
    
    bool method enabledByDefault()
    {
        return true;
    }


} /* namespace Gear */
