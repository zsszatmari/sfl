//
//  SortDescriptor.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/25/13.
//
//

#ifndef __G_Ear__SortDescriptor__
#define __G_Ear__SortDescriptor__

#include <vector>
#include <string>
#include "Environment.h"

namespace ClientDb
{
    class SortDescriptor;
}

namespace Gear
{
    using std::vector;
    using std::string;
    using std::pair;
    
    class SortDescriptor final
    {
    public:
        SortDescriptor();
        SortDescriptor(const vector<pair<string, bool>> &keysAndAscending);
        SortDescriptor(const SortDescriptor &rhs);
        explicit SortDescriptor(const string &serialized);
        string serialize() const;
        operator ClientDb::SortDescriptor () const;

        SortDescriptor &operator=(const SortDescriptor &rhs);
        explicit operator bool() const;
        bool operator==(const SortDescriptor &rhs) const;
        
        const vector<pair<string, bool>> &keysAndAscending() const;
        
    private:
        vector<pair<string, bool>> _keysAndAscending;
    };
}
#endif /* defined(__G_Ear__SortDescriptor__) */
