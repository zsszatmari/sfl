//
//  GearImageView.h
//  G-Ear iOS
//
//  Created by Gál László on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "stdplus.h"

namespace Gui
{
    class IPaintable;
    class Color;
}
namespace Gear
{
    class PromisedImage;
}

@interface GearImageView : UIView

@property(nonatomic, assign) BOOL stretchImage;
@property(nonatomic, assign) shared_ptr<Gui::IPaintable> image;
@property(nonatomic, assign) shared_ptr<Gear::PromisedImage> promise;
- (void)setTintColor:(Gui::Color)color;

@end
