//
//  ImageToDeliver.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__ImageToDeliver__
#define __G_Ear_Player__ImageToDeliver__

#include "PromisedImage.h"

namespace Gear
{
    class ImageToDeliver final : public PromisedImage
    {
    public:
        ImageToDeliver(long requestDimension);

        enum class Rating {
            Nothing = 0,
            NoArt = 1,
            BasedOnArtistOnly = 2,
            Provided = 3,
            FetchedFromCache = 4,
            ProperLastFm = 6
        };
        
        void setImage(const shared_ptr<Gui::IPaintable> &image, Rating rating);
        long requestedDimension() const;
        void setRequestedDimension(long requestedDimension);

    private:
        long _requestedDimension;
        Rating _rating;       
    };
}

#endif /* defined(__G_Ear_Player__ImageToDeliver__) */
