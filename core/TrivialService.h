/*
 * TrivialService.h
 *
 *  Created on: Nov 6, 2013
 *      Author: zsszatmari
 */

#ifndef TRIVIALSERVICE_H_
#define TRIVIALSERVICE_H_

#include "PreferenceService.h"

namespace Gear
{
	class SessionManager;
	class ISession;

	class TrivialService : public PreferenceService
	{
	public:
		virtual string title() const = 0;

	protected:
        TrivialService(shared_ptr<SessionManager> &sessionManager);

	private:
        virtual void connected();
        virtual void disconnected();
        virtual shared_ptr<ISession> createSession() = 0;
        virtual bool enabledByDefault();

        weak_ptr<ISession> _activeSession;
        shared_ptr<SessionManager> _sessionManager;
	};

} /* namespace Gear */

#endif /* TRIVIALSERVICE_H_ */
