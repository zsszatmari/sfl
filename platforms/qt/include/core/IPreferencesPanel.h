//
//  IPreferencesPanel.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/26/13.
//
//

#ifndef __G_Ear_Player__IPreferencesPanel__
#define __G_Ear_Player__IPreferencesPanel__

#include <vector>
#include "Environment.h"

//#define OPTION_DISABLE_INAPP

namespace Gear
{
    using std::vector;
    
    class Preference;
    class PreferenceGroup;
    
    class core_export IPreferencesPanel
    {
    public:
        virtual vector<PreferenceGroup> preferenceGroups() const = 0;
        
        virtual ~IPreferencesPanel();
        
#ifdef OPTION_DISABLE_INAPP
        static void disableInApp();
#endif
    };
}

#endif /* defined(__G_Ear_Player__IPreferencesPanel__) */
