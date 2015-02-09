//
//  SortDescriptor.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/25/13.
//
//

#include "SortDescriptor.h"
#include "SongSortOrder.h"
#include "sfl/Prelude.h"
#include "DbSortDescriptor.h"

namespace Gear {
    
#define method SortDescriptor::

    using namespace sfl;
    
    method SortDescriptor()
    {
    }
    
    method SortDescriptor(const vector<pair<string, bool>> &keysAndAscending)
        : _keysAndAscending(keysAndAscending)
    {
    }
    
    method SortDescriptor(const SortDescriptor &rhs)
        : _keysAndAscending(rhs._keysAndAscending)
    {
    }

    method SortDescriptor(const string &serialized)
    {
        if (serialized.empty()) {
            *this = SongSortOrder::sortDescriptor();
            return;
        }
        bool ascending = serialized[0] == 'A';
        string key = serialized.substr(1);
        *this = SongSortOrder::sortDescriptor(key, ascending);
    }

    string method serialize() const
    {
        if (_keysAndAscending.empty()) {
            return "";
        }   
        auto p = _keysAndAscending.front();
        string ret = p.first;
        if (p.second) {
            // ascending
            ret.insert(0, "A");
        } else {
            ret.insert(0, "D");
        }
        return ret;
    }
    
    SortDescriptor & method operator=(const SortDescriptor &rhs)
    {
        _keysAndAscending = rhs._keysAndAscending;
        return *this;
    }
    
    const vector<pair<string, bool>> & method keysAndAscending() const
    {
        return _keysAndAscending;
    }
    
    bool method operator==(const SortDescriptor &rhs) const
    {
        return _keysAndAscending == rhs._keysAndAscending;
    }
    
    method operator bool() const
    {
        return !_keysAndAscending.empty();
    }

    method operator ClientDb::SortDescriptor () const
    {
        return _keysAndAscending.empty() 
                ? ClientDb::SortDescriptor() 
                : ClientDb::SortDescriptor(
                    map(&fst<string,bool>, _keysAndAscending),
                    _keysAndAscending.at(0).second);
    }    
}