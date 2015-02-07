//
//  IAlbum.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#ifndef __G_Ear__IAlbum__
#define __G_Ear__IAlbum__

#include <string>

namespace Gear
{
    using std::string;
    
    class IAlbum
    {
    public:
        virtual ~IAlbum();
        virtual bool operator==(const IAlbum &rhs) const = 0;
        
        virtual const string uniqueId() const = 0;
    };
}

#endif /* defined(__G_Ear__IAlbum__) */
