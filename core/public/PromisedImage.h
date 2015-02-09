//
//  PromisedImage.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__PromisedImage__
#define __G_Ear_Player__PromisedImage__

#include "ManagedValue.h"

namespace Gui
{
    class IPaintable;
}

namespace Gear
{
    class core_export PromisedImage
    {
    public:
        PromisedImage();
        virtual ~PromisedImage();
        Base::ValueConnector<shared_ptr<Gui::IPaintable>> connector();
        shared_ptr<Gui::IPaintable> image();
        
    protected:
        Base::ManagedValue<shared_ptr<Gui::IPaintable>> _image;
        
    private:
        PromisedImage(const PromisedImage &rhs); // delete
        PromisedImage &operator=(const PromisedImage &rhs); // delete
    };
    
}

#endif /* defined(__G_Ear_Player__PromisedImage__) */
