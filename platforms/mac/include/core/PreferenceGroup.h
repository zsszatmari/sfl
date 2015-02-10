//
//  PreferenceGroup.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/26/13.
//
//

#ifndef __G_Ear_Player__PreferenceGroup__
#define __G_Ear_Player__PreferenceGroup__

#include <vector>
#include <string>
#include "Environment.h"

namespace Gear {
    
    using std::vector;
    using std::string;
    
    class Preference;
    
    class core_export PreferenceGroup final
    {
    public:
        PreferenceGroup(const string &title,const vector<Preference> &prefs, const vector<PreferenceGroup> &subgroups = vector<PreferenceGroup>() );
        const vector<Preference> &preferences() const;
        const vector<PreferenceGroup> &subgroups();
        const string & title() const;
        
    private:
        string _title;
        vector<Preference> _preferences;
        vector<PreferenceGroup> _subgroups;
    };
}

#endif /* defined(__G_Ear_Player__PreferenceGroup__) */
