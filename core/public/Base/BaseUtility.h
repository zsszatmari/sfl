//
//  Utility.h
//  Base
//
//  Created by Zsolt Szatmári on 5/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef Base_Utility_h
#define Base_Utility_h

#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include "Environment.h"

#ifdef USE_BOOST_LOCALES
#include "boost/locale.hpp"
#else
//#include <locale>
#endif
#include "stdplus.h"

namespace Base {

    using std::string;    
    
    template<typename C, typename T, int size>
    C init(T(&arr)[size])
    {
        return C(arr, arr+size);
    }
    
#ifdef USE_BOOST_LOCALES
    extern std::locale utfLocale;
#endif
    void localeInit();
    
    inline string toLower(const string &rhs)
    {
#ifdef USE_BOOST_LOCALES
        return boost::locale::to_lower(rhs, utfLocale);
#else
        // this could be problematic for sorting multibyte characters...
        string ret = rhs;
        std::transform(ret.begin(), ret.end(),
                       ret.begin(), ::tolower);
        return ret;
#endif
    }
    
    inline void copyToLower(char *dst, const char *src)
    {
        while (*src != '\0') {
            *dst = tolower(*src);
            ++dst;
            ++src;
        }
        // only copy!
        //*dst = '\0';
    }
 
    // set and overwrite
    template<class T> void mapSetValue(T &m, const typename T::key_type &key, const typename T::mapped_type &value)
    {
        auto result = m.insert(make_pair(key, value));
        if (!result.second) {
            result.first->second = value;
        }
    }
    
    template<typename T>
    bool keyCompareL(const std::pair<string, T> &lhs, const string &rhs)
    {
        return lhs.first < rhs;
    }
    
    template<typename T>
    bool keyCompareR(const std::string &lhs, const std::pair<string, T> &rhs)
    {
        return lhs < rhs.first;
    }
    
    template<typename T>
    bool keyCompare(const std::pair<string, T> &lhs, const std::pair<string, T> &rhs)
    {
        return lhs.first < rhs.first;
    }
    
/*    template<typename T>
    std::vector<std::pair<string,T>> mapToSortedVector(const unordered_map<string, T> &attributes)
    {
        std::vector<std::pair<string,T>> ret;
        ret.reserve(attributes.size());
        copy(attributes.begin(), attributes.end(), back_inserter(ret));
        sort(ret.begin(), ret.end(), keyCompare<T>);
        return ret;
    }*/
}


#endif
