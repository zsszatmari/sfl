//
//  GearUtility.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/20/13.
//
//

#ifndef G_Ear_GearUtility_h
#define G_Ear_GearUtility_h

#include <string>
#include "stdplus.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear {

    template<typename T>
    const T &empty()
    {
        static T instance;
        return instance;
    }
    
    // equality based on content
    template<typename T>
    const bool equals(const shared_ptr<T> lhs, const shared_ptr<T>rhs )
    {
        return (lhs.get() == rhs.get()) || (lhs && rhs && (*lhs == *rhs));
    }
    
    const std::string random_string(size_t length);
}


#endif
