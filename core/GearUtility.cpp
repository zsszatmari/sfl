//
//  GearUtility.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#include <stdlib.h>
#include <algorithm>
#include "GearUtility.h"

namespace Gear
{
    const std::string random_string(size_t length)
    {
        auto randchar = []() -> char
        {
            const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[ rand() % max_index ];
        };
        std::string str(length,0);
        std::generate_n( str.begin(), length, randchar );
        return str;
    }
}