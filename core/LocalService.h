/*
 * LocalService.h
 *
 *  Created on: Nov 6, 2013
 *      Author: zsszatmari
 */

#ifndef LOCALSERVICE_H_
#define LOCALSERVICE_H_

#include "TrivialService.h"

namespace Gear
{
	class LocalService : public TrivialService
	{
	public:
		static shared_ptr<LocalService> create(shared_ptr<SessionManager> &sessionManager);
        static bool available();
        virtual string inAppIdentifier() const;
        virtual bool inAppNecessary() const;

	private:
		LocalService(shared_ptr<SessionManager> &sessionManager);

		virtual string title() const;
		virtual string preferencesKey() const;
		virtual shared_ptr<ISession> createSession();
	};

} /* namespace Gear */
#endif /* LOCALSERVICE_H_ */
