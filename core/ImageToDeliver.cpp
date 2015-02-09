//
//  ImageToDeliver.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#include "ImageToDeliver.h"

namespace Gear
{
#define method ImageToDeliver::
    
    method ImageToDeliver(long requestedDimension) :
        _requestedDimension(0),
        _rating(Rating::Nothing)
    {
    }
    
    long method requestedDimension() const
    {
        return _requestedDimension;
    }
    
    void method setRequestedDimension(long requestedDimension)
    {
        _requestedDimension = requestedDimension;
    }
    
    void method setImage(const shared_ptr<Gui::IPaintable> &image, ImageToDeliver::Rating rating)
    {
        if (static_cast<int>(rating) >= static_cast<int>(_rating)) {
            _image = image;
            _rating = rating;
        }
    }
}