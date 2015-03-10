//
//  GearImageView.m
//  G-Ear iOS
//
//  Created by Gál László on 7/30/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import "GearImageView.h"
#include "SignalConnection.h"
#include "PromisedImage.h"
#include "Painter.h"
#include "Tinter.h"
#include "BitmapImage.h"
#include "NamedImage.h"
#include "StringUtility.h"

@implementation GearImageView {
    shared_ptr<Gear::PromisedImage> _promise;
    shared_ptr<Gui::IPaintable> _image;
    shared_ptr<Gui::IPaintable> _oldImage;
    Base::SignalConnection promiseConnection;
    BOOL stretch;
}

- (id)initWithFrame:(CGRect)aRect
{
    self = [super initWithFrame:aRect];
    if (self) {
        self.backgroundColor = [UIColor clearColor];
        _stretchImage = YES;
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)decoder
{
    self = [super initWithCoder:decoder];
    if (self) {
        self.backgroundColor = [UIColor clearColor];
        _stretchImage = YES;
    }
    return self;
}

- (void)setPromise:(shared_ptr<Gear::PromisedImage>)aPromise
{
    _oldImage = nullptr;
    _image = nullptr;
    _promise = aPromise;
	__weak GearImageView *wself = self;
    if (_promise) {
        promiseConnection = _promise->connector().connect([wself](const shared_ptr<Gui::IPaintable> &paintable) {
            [wself setImage:paintable];
        });
    }
}

- (shared_ptr<Gear::PromisedImage>)promise
{
    return _promise;
}

- (void)setImage:(shared_ptr<Gui::IPaintable>)aImage
{
    _oldImage = _image;
    _image = aImage;
    [self setNeedsDisplay];
}
                                                         
- (shared_ptr<Gui::IPaintable>)image
{
    return _image;
}

- (void)drawRect:(CGRect)rect
{
    if (self.image) {
        
        if (!Gui::Painter::paint(self.image, self.stretchImage, self.bounds)) {
            double delayInSeconds = 0.5;
            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
            __weak GearImageView *wself = self;
            
            dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
                [wself setNeedsDisplay];
            });
            
            if (_oldImage) {
                Gui::Painter::paint(_oldImage, self.stretchImage, self.bounds);
            }
        } else {
            _oldImage = nullptr;
        }
    }
}

- (void)setTintColor:(Gui::Color)color
{
    using namespace Gui;
    
    self.image = Tinter::tintWithColor(_image, color);
}

/*
- (CGSize)intrinsicContentSize
{
    auto named = dynamic_pointer_cast<Gui::NamedImage>(_image);
    if (named) {
        auto name = Gear::convertString(named->imageName());
        auto size = [UIImage imageNamed:name].size;
        return size;
    }
    return CGSizeMake(UIViewNoIntrinsicMetric, UIViewNoIntrinsicMetric);
}*/

@end
