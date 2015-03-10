//
//  ApplePreferences.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/16/13.
//
//

#import <Foundation/Foundation.h>
#include "StringUtility.h"
#include "ApplePreferences.h"

namespace Gear
{
#define method ApplePreferences::

    bool method boolForKey(const string &key) const
    {
        @autoreleasepool {
            return [[NSUserDefaults standardUserDefaults] boolForKey:convertString(key)];
        }
    }
    
    unsigned long method uintForKey(const string &key) const
    {
        @autoreleasepool {
            return [[NSUserDefaults standardUserDefaults] integerForKey:convertString(key)];
        }
    }
    
    void method setUintForKey(const string &key, const uint64_t value)
    {
        @autoreleasepool {
            [[NSUserDefaults standardUserDefaults] setInteger:value forKey:convertString(key)];
        }
    }

    string method stringForKey(const string &key) const
    {
        @autoreleasepool {
            return convertString([[NSUserDefaults standardUserDefaults] stringForKey:convertString(key)]);
        }
    }

    void method setStringForKey(const string &key, const string &value)
    {
        @autoreleasepool {
            [[NSUserDefaults standardUserDefaults] setObject:convertString(value) forKey:convertString(key)];
        }
    }

    float method floatForKey(const string &key) const
    {
        @autoreleasepool {
            return [[NSUserDefaults standardUserDefaults] floatForKey:convertString(key)];
        }
    }

    void method setFloatForKey(const string &key, const float value)
    {
        @autoreleasepool {
            [[NSUserDefaults standardUserDefaults] setFloat:value forKey:convertString(key)];
        }
    }

}