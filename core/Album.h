//
//  Album.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#ifndef __G_Ear__Album__
#define __G_Ear__Album__

#include "IAlbum.h"

namespace Gear
{
    using std::string;
    
    class Album : public IAlbum
    {
    public:
        Album(const string uniqueId);
        virtual const string uniqueId() const;
        virtual bool operator==(const IAlbum &rhs) const;
        
    private:
        const string _uniqueId;
    };
}

#endif /* defined(__G_Ear__Album__) */
