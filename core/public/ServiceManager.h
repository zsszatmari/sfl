//
//  ServiceManager.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#ifndef __G_Ear_Player__ServiceManager__
#define __G_Ear_Player__ServiceManager__

#include <vector>
#include "stdplus.h"
#include "IService.h"

namespace Gear
{
    using std::vector;
    
    class IService;
    class SessionManager;
    
    class core_export ServiceManager final
    {
    public:
        ServiceManager(shared_ptr<SessionManager> &sessionManager);
        vector<shared_ptr<IService>> services() const;
        void autoconnect();
        IService::State generalState() const;
        
        const shared_ptr<IService> googlePlayService() const;
        
    private:
        vector<shared_ptr<IService>> _services;
        const shared_ptr<SessionManager> _sessionManager;
    };
}

#endif /* defined(__G_Ear_Player__ServiceManager__) */
