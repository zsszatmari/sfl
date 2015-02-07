//
//  GooglePlayService.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#ifndef __G_Ear_Player__GooglePlayService__
#define __G_Ear_Player__GooglePlayService__

#include "stdplus.h"
#include "IService.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    class SessionManager;
    class DbSession;
    
    class GooglePlayService final : public IService, public MEMORY_NS::enable_shared_from_this<GooglePlayService>
    {
    public:
        static string identifier();
        static shared_ptr<GooglePlayService> create(shared_ptr<SessionManager> &sessionManager);
        
        virtual string title() const;
        // connect only if credentials are saved
        virtual void autoconnect();
        // try to connect, possibly asking for credentials
        virtual void connect();
        virtual void disconnect();
        
        virtual string inAppIdentifier() const;
        virtual bool inAppNecessary() const;
        
    private:
        GooglePlayService(shared_ptr<SessionManager> &serviceManager);
        
        void disconnect(bool deleteCredentials);

        shared_ptr<SessionManager> _sessionManager;
        weak_ptr<DbSession> _activeSession;
    };
}

#endif /* defined(__G_Ear_Player__GooglePlayService__) */
