//
//  IArtist.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#ifndef __G_Ear__IArtist__
#define __G_Ear__IArtist__

#include <string>

namespace Gear
{
    using std::string;
    
    class IArtist
    {
    public:
        virtual ~IArtist();
        
        virtual const string uniqueId() const = 0;
    };
}

#endif /* defined(__G_Ear__IArtist__) */
