//
//  SpotifyService.h
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/13/13.
//
//

#ifndef __G_Ear_Player__SpotifyService__
#define __G_Ear_Player__SpotifyService__

#include "IService.h"

#ifndef TIZEN
#define DISABLE_SPOTIFY
#endif

namespace Gear
{
    class SpotifySession;
    class SessionManager;
    
#ifndef DISABLE_SPOTIFY
    
    class SpotifyService : public IService, public MEMORY_NS::enable_shared_from_this<SpotifyService>
    {
    public:
        static shared_ptr<SpotifyService> create(shared_ptr<SessionManager> &sessionManager);
        
        virtual string title() const;
        // connect only if credentials are saved
        virtual void autoconnect();
        // try to connect, possibly asking for credentials
        virtual void connect();
        virtual void disconnect();
        
    private:
        SpotifyService(shared_ptr<SessionManager> &sessionManager);
        
        void connnectSucceeded();

        shared_ptr<SpotifySession> _session;
        shared_ptr<SessionManager> _sessionManager;
    };
    
#else
    
    class SpotifyService : IService
    {
    public:
        virtual string title() const {return "";}
        // connect only if credentials are saved
        virtual void autoconnect() {}
        // try to connect, possibly asking for credentials
        virtual void connect() {}
        virtual void disconnect() {}
    };
#endif
    

}

#endif /* defined(__G_Ear_Player__SpotifyService__) */
