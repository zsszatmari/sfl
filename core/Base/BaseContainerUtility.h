//
//  BaseContainerUtility.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/12/13.
//
//

#ifndef G_Ear_Player_BaseContainerUtility_h
#define G_Ear_Player_BaseContainerUtility_h

#include <algorithm>
#include "stdplus.h"

namespace Base
{

    template <class T> unordered_map<string, typename T::value_type> toUnorderedMap(const T &c)
    {
        unordered_map<string, typename T::value_type> underConstruction;
        std::transform(c.begin(), c.end(), inserter(underConstruction, underConstruction.begin()), [](typename T::value_type object){
            return make_pair(object.name(), object);
        });
        
        return underConstruction;
    }

    template <class K, class V> const vector<K> allKeys(const unordered_map<K, V> &m)
    {
        vector<K> ret;
        ret.reserve(m.size());
        for (auto it = m.begin() ; it != m.end() ; ++it) {
            ret.push_back(it->first);
        }
        
        return ret;
    }

    template <class K, class V> const vector<V> allValues(const unordered_map<K, V> &m)
    {
        vector<V> ret;
        ret.reserve(m.size());
        for (auto it = m.begin() ; it != m.end() ; ++it) {
            //ret.push_back(EntityDescription());
            ret.push_back(it->second);
        }
        
        return ret;
    }

    template <class T> const set<T> vectorToSet(const vector<T> &v)
    {
        return set<T>(v.begin(), v.end());
    }

    template <class T> set<string> allNames(set<T> s)
    {
        set<string> ret;
        for (auto it = s.begin() ; it != s.end() ; ++it) {
            //ret.emplace_hint(ret.end(), element.name());
            ret.insert(ret.end(), it->name());
        }
        
        return ret;
    }
}
    
#endif
