//
//  PromisedImage.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#include "PromisedImage.h"

namespace Gear
{
#define method PromisedImage::
    
    method PromisedImage()
    {
    }
    
    method ~PromisedImage()
    {
    }
    
    Base::ValueConnector<shared_ptr<Gui::IPaintable>> method connector()
    {
        return _image.connector();
    }
    
    
    shared_ptr<Gui::IPaintable> method image()
    {
        return _image;
    }
}