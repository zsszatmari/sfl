//
//  ISongIntent.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/30/13.
//
//

#ifndef __G_Ear__ISongIntent__
#define __G_Ear__ISongIntent__

#include <vector>
#include <string>
#include "Environment.h"

namespace Gear
{
    using std::vector;
    using std::string;
    
    class SongEntry;
    
    class core_export ISongIntent
    {
    public:
        virtual ~ISongIntent();
        
        virtual const string menuText() const = 0;
        virtual void apply(const vector<SongEntry> &songs) const = 0;
        virtual bool confirmationNeeded(const vector<SongEntry> &songs) const;
        virtual string confirmationText(const vector<SongEntry> &songs) const;
    };
}

#endif /* defined(__G_Ear__ISongIntent__) */
