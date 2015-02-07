//
//  OfflineService.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 02/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "OfflineService.h"
#include "IApp.h"
#include "ISession.h"
#include "IPreferences.h"
#include "PreferencesPanel.h"
#include "SessionManager.h"
#include "ServiceManager.h"
#include "GooglePlayService.h"

namespace Gear
{
#define method OfflineService::
    
    static OfflineService *instance = nullptr;
    
    method OfflineService()
    {
        instance = this;
    }
    
    string method title() const
    {
        return "Offline Mode";
    }
    
    void method connected()
    {
        IApp::instance()->sessionManager()->setOffline(true);
    }
    
    void method disconnected()
    {
        
        IApp::instance()->sessionManager()->setOffline(false);
    }
    
    static const string kPreferencesKey = "OfflineMode";
    
    std::string method preferencesKey() const
    {
        return kPreferencesKey;
    }
    
    bool method offlineMode()
    {
        return IApp::instance()->preferences().boolForKey(kPreferencesKey);
    }
    
    string method inAppIdentifier() const
    {
        return "offline";
    }
    
    bool method inAppNecessary() const
    {
        // not permitted to download from youtube due to licensing problems.
        // offline for google music alone would be too little for charging $1
        return false;
    }
    
    bool method available()
    {
        if (!instance) {
            return false;
        }
        return PreferencesPanel::checkImmediately(*instance);
    }
    
    bool method disabledStatic()
    {
        auto serviceManager = IApp::instance()->serviceManager();
        if (!serviceManager) {
            return false;
        }
        return !PreferencesPanel::checkImmediately(*serviceManager->googlePlayService().get());
        // this is wrong: if I purchased google music, but logged out (maybe can't log in because there is no internet at all!, I still want to use offline mode
        //return IApp::instance()->serviceManager()->googlePlayService()->state() != IService::State::Online;
    }
    
    bool method disabled() const
    {
        return disabledStatic();
    }
}
