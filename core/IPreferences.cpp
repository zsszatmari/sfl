//
//  IPreferences.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/16/13.
//
//

#include "IPreferences.h"

namespace Gear
{
#define method IPreferences::
    
    method IPreferences()
    {
    }

    method ~IPreferences()
    {
    }

    string method stringForKey(const string &key) const
    {
        return "";
    }

    void method setStringForKey(const string &key, const string &value)
    {
    }
}