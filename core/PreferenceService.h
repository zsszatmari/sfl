//
//  PreferenceService.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 02/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__PreferenceService__
#define __G_Ear_core__PreferenceService__

#include "IService.h"

namespace Gear
{
    class PreferenceService : public IService
    {
    public:
        virtual void autoconnect();
        virtual void connect();
        virtual void disconnect();
        
    private:
        virtual void connected() = 0;
        virtual void disconnected() = 0;
        virtual bool enabledByDefault();
        virtual string preferencesKey() const = 0;
    };
}

#endif /* defined(__G_Ear_core__PreferenceService__) */
