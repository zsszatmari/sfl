//
//  StringUtility.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/22/13.
//
//

#ifndef G_Ear_StringUtility_h
#define G_Ear_StringUtility_h

#import <Foundation/Foundation.h>
#include <map>
#include "GearUtility.h"
#include "stdplus.h"
#include "BaseUtility.h"

namespace Gear
{
    using std::string;
    using namespace Base;
    
    inline
    std::string convertString(NSString *rhs)
    {
        auto ptr = [rhs cStringUsingEncoding:NSUTF8StringEncoding];
        if (ptr) {
            return std::string(ptr);
        } else {
            return std::string();
        }
    }
    
    inline
    std::string convertString(NSData *rhs)
    {
        auto ptr = [rhs bytes];
        if (ptr) {
            return std::string(static_cast<const char *>(ptr), [rhs length]);
        } else {
            return std::string();
        }
    }
    
    /*
    inline
    string convertString16(NSString *rhs)
    {
        auto ptr = (char16_t *)[rhs cStringUsingEncoding:NSUTF16StringEncoding];
        if (ptr) {
            return string(ptr);
        } else {
            return string();
        }
    }*/
    
    inline
    NSString *convertString(const std::string &rhs)
    {
        return [NSString stringWithCString:rhs.c_str() encoding:NSUTF8StringEncoding];
    }
    
    /*
    inline
    NSString *convertString(const string &rhs)
    {
        return [NSString stringWithCString:utf16ToUtf8(rhs).c_str() encoding:NSUTF8StringEncoding];
        // this did not seem to be working:
        //return [NSString stringWithCString:reinterpret_cast<const char *>(rhs.c_str()) encoding:NSUTF16StringEncoding];
    }*/
    
    inline NSDictionary *convertStringMap(const std::map<string, string> &m)
    {
        NSMutableDictionary *dictionary = [NSMutableDictionary dictionary];
        for (auto &p : m) {
            [dictionary setObject:convertString(p.second) forKey:convertString(p.first)];
        }
        return dictionary;
    }
    
    inline const std::map<string, string> convertStringMap(NSDictionary *dictionary)
    {
        __block std::map<string, string> m;
        [dictionary enumerateKeysAndObjectsUsingBlock:^(NSString *key, NSString *obj, BOOL *stop){
            
            if ([obj isKindOfClass:[NSString class]]) {
                m.insert(make_pair(convertString(key), convertString(obj)));
            }
        }];
        return m;
    }
}
#endif
