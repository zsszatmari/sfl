//
//  PreferenceGroup.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/26/13.
//
//

#include "PreferenceGroup.h"
#include "Preference.h"

namespace Gear
{
#define method PreferenceGroup::
    
    method PreferenceGroup(const string &title, const vector<Preference> &prefs, const vector<PreferenceGroup> &subgroups) :
        _title(title),
        _preferences(prefs),
        _subgroups(subgroups)
    {
    }
    
    const vector<Preference> & method preferences() const
    {
        return _preferences;
    }
    
    const vector<PreferenceGroup> & method subgroups()
    {
        return _subgroups;
    }
    
    const string & method title() const
    {
        return _title;
    }
}