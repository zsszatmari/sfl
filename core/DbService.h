//
//  GooglePlayService.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#ifndef __G_Ear_Player__DbService__
#define __G_Ear_Player__DbService__

#include "stdplus.h"
#include "IService.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    class SessionManager;
    class DbSession;
    
    class DbService : public IService, public MEMORY_NS::enable_shared_from_this<DbService>
    {
    public:
        virtual string title() const = 0;
        // connect only if credentials are saved
        virtual void autoconnect();
        // try to connect, possibly asking for credentials
        virtual void connect();
        virtual void disconnect();
        
        virtual string identifier() = 0;
        virtual string inAppIdentifier() const = 0;
        virtual bool inAppNecessary() const = 0;
        DbService(const shared_ptr<SessionManager> &serviceManager);
        
    private:
        virtual shared_ptr<DbSession> createSession() = 0;

        void disconnect(bool deleteCredentials);

        shared_ptr<SessionManager> _sessionManager;
        weak_ptr<DbSession> _activeSession;
    };
}

#endif /* defined(__G_Ear_Player__GooglePlayService__) */
