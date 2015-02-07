//
//  OfflineService.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 02/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__OfflineService__
#define __G_Ear_core__OfflineService__

#include "PreferenceService.h"

namespace Gear
{
    class OfflineService final : public PreferenceService
    {
    public:
        OfflineService();
        
        virtual string title() const;
        // connect only if credentials are saved
        
        static bool offlineMode();
        virtual string inAppIdentifier() const;
        virtual bool inAppNecessary() const;
        static bool available();
        static bool disabledStatic();
        virtual bool disabled() const;
        
    private:
        virtual void connected();
        virtual void disconnected();
        virtual std::string preferencesKey() const;
    
    };
}

#endif /* defined(__G_Ear_core__OfflineService__) */
