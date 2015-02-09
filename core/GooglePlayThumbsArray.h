//
//  GooglePlayThumbsArray.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/7/13.
//
//

#ifndef __G_Ear_Player__GooglePlayThumbsArray__
#define __G_Ear_Player__GooglePlayThumbsArray__

#include "GooglePlayNonLocalArray.h"

namespace Gear
{
    class GooglePlayThumbsArray : public GooglePlayNonLocalArray, public MEMORY_NS::enable_shared_from_this<GooglePlayThumbsArray>
    {
    public:
        static shared_ptr<GooglePlayThumbsArray> create(const shared_ptr<GooglePlaySession> &session);
        virtual const bool orderedArray() const;
        
    private:
        GooglePlayThumbsArray(const shared_ptr<GooglePlaySession> &session);
        
        void fetch();
    };

}

#endif /* defined(__G_Ear_Player__GooglePlayThumbsArray__) */
