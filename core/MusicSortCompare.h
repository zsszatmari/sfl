#ifndef G_Ear_Player_MusicSortCompare_h
#define G_Ear_Player_MusicSortCompare_h

#include "StringCompare.h"

namespace Gear
{
    using std::string;

    class MusicSortCompare final
    {
    public:
        // string-only greaterThan would mean trouble, it works very differently
        static bool lowerThan(const std::string &lhs, const std::string &rhs);
        static bool greaterThan(const std::string &lhs, const std::string &rhs);
    };
    
#define method MusicSortCompare::
    
    inline bool method lowerThan(const std::string &lhs, const std::string &rhs)
    {
        return StringCompare::compare(lhs.c_str(), rhs.c_str()) < 0;
        //return lhs < rhs;
    }

    inline bool method greaterThan(const std::string &lhs, const std::string &rhs)
    {
        return lowerThan(rhs, lhs);
    }
    
#undef method
}

#endif