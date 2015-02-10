//
//  IPreferences.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/16/13.
//
//

#ifndef __G_Ear_Player__IPreferences__
#define __G_Ear_Player__IPreferences__

#include <string>
#include <stdint.h>
#include "Environment.h"

namespace Gear
{
    using std::string;
    
    class core_export IPreferences
    {
    public:
        IPreferences();
        virtual ~IPreferences();

        virtual bool boolForKey(const string &key) const = 0;
        virtual unsigned long uintForKey(const string &key) const = 0;
        virtual void setUintForKey(const string &key, const uint64_t value) = 0;
        
        virtual float floatForKey(const string &key) const = 0;
        virtual void setFloatForKey(const string &key, const float value) = 0;

        virtual string stringForKey(const string &key) const = 0;
        virtual void setStringForKey(const string &key, const string &value) = 0;

    private:
        IPreferences &operator=(const IPreferences &); // delete
        IPreferences(const IPreferences &); // delete
    };
}

#endif /* defined(__G_Ear_Player__IPreferences__) */
