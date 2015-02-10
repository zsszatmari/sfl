//
//  PreferencesPanel.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/13/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__PreferencesPanel__
#define __G_Ear_core__PreferencesPanel__

#include "IPreferencesPanel.h"
#include "Environment.h"
#include "stdplus.h"
#include <functional>

namespace Gear
{
    class IService;
  	using std::function;
    
    class PreferencesPanel final : public IPreferencesPanel
    {
    public:
        virtual vector<PreferenceGroup> preferenceGroups() const;
        
        static void checkService(const Gear::IService &service, const function<void()> &success , const function<void()> &fail);
        static bool checkImmediately(const Gear::IService &service);
    };;
}

#endif /* defined(__G_Ear_core__PreferencesPanel__) */
