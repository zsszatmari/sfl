//
//  YouTubeService.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/23/13.
//
//

#ifndef __G_Ear_Player__YouTubeService__
#define __G_Ear_Player__YouTubeService__

#include "IService.h"
#include "CoreDebug.h"
#include SHAREDFROMTHIS_H

namespace Gear
{
    class OAuthConnection;
    class SessionManager;
    class ISession;
    
    class YouTubeService final : public IService, public MEMORY_NS::enable_shared_from_this<YouTubeService>
    {
    public:
        static shared_ptr<YouTubeService> create(shared_ptr<SessionManager> &sessionManager);
        
        
        virtual string title() const;
        // connect only if credentials are saved
        virtual void autoconnect();
        // try to connect, possibly asking for credentials
        virtual void connect();
        virtual void disconnect();
        
        virtual string inAppIdentifier() const;
        virtual bool inAppNecessary() const;
        
    private:
        YouTubeService(shared_ptr<SessionManager> &sessionManager);
        
        void connected();
        void resetConnection();
        
        shared_ptr<OAuthConnection> _connection;
        weak_ptr<ISession> _activeSession;
        shared_ptr<SessionManager> _sessionManager;
    };
}

#endif /* defined(__G_Ear_Player__YouTubeService__) */
