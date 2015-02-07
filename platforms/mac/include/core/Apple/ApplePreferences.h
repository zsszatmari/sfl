//
//  ApplePreferences.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/16/13.
//
//

#ifndef __G_Ear_Player__ApplePreferences__
#define __G_Ear_Player__ApplePreferences__

#include "IPreferences.h"

namespace Gear
{
    class ApplePreferences final : public IPreferences
    {
    public:
        virtual bool boolForKey(const string &key) const;
        virtual unsigned long uintForKey(const string &key) const;
        virtual void setUintForKey(const string &key, const uint64_t value);

        virtual string stringForKey(const string &key) const;
        virtual void setStringForKey(const string &key, const string &value);

	 	virtual float floatForKey(const string &key) const;
        virtual void setFloatForKey(const string &key, const float value);

    };
}

#endif /* defined(__G_Ear_Player__ApplePreferences__) */
