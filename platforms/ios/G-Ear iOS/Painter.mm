//
//  Painter.cpp
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "Painter.h"
#include "IPaintable.h"
#include "Color.h"
#include "NamedImage.h"
#include "BitmapImage.h"
#include "StringUtility.h"
#include "Gradient.h"
#include "SerialExecutor.h"
#include "MainExecutor.h"
#include <sstream>
#include WEAK_H

namespace Gui
{
#define method Painter::
    
    bool method paint(const shared_ptr<IPaintable> &paintable, bool stretch)
    {
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        return paint(paintable, stretch, CGRectMake(0, 0, CGBitmapContextGetWidth(ctx), CGBitmapContextGetHeight(ctx)));
    }
    
    bool method paint(const shared_ptr<IPaintable> &paintable, bool stretch, CGRect rect)
    {        
        Painter painter(rect, stretch, paintable);
        paintable->paint(painter);
        return painter._success;
    }
    
    method Painter(CGRect rect, bool stretch, const shared_ptr<IPaintable> &paintable) :
        _rect(rect),
        _stretch(stretch),
        _success(true),
        _paintable(paintable),
        _converting(false),
        _wasConverted(false),
        _converted(nil)
    {
    }
    
    void method paint(const Color &color) const
    {
        if (_callIfCantConvert) {
            _callIfCantConvert();
        }
        
        [[UIColor colorWithRed:color.red() green:color.green() blue:color.blue() alpha:color.alpha()] setFill];
        UIRectFill(_rect);
    }
    
    UIColor * method convertColor(const Color &color)
    {
        return [UIColor colorWithRed:color.red() green:color.green() blue:color.blue() alpha:color.alpha()];
    }
    
    UIImage * method convertImage(const shared_ptr<IPaintable> &paintable, CGSize size)
    {
        if (!paintable) {
            return nil;
        }
        
        // this is to be called later unless _wasConverted = true
        //UIGraphicsBeginImageContextWithOptions(size, NO, 0.0f);
        
        // might be double size... but this has already been taken care of
        //CGContextRef ctx = UIGraphicsGetCurrentContext();
        //CGRect rect = CGRectMake(0, 0, CGBitmapContextGetWidth(ctx), CGBitmapContextGetHeight(ctx));
        CGRect rect = CGRectMake(0, 0, size.width, size.height);
        Painter painter(rect, false, paintable);
        painter._converting = true;
        painter._callIfCantConvert = [size,paintable] () mutable {
            if (size.width == 0 || size.height == 0) {
                // ios 6 specific hack
                auto named = dynamic_pointer_cast<NamedImage>(paintable);
                size = [UIImage imageNamed:Gear::convertString(named->imageName())].size;
                size.width *= 2;
                size.height *= 2;
            }
            UIGraphicsBeginImageContextWithOptions(size, NO, 0.0f);
        };
        
        paintable->paint(painter);
        
        if (painter._wasConverted) {
            if (!painter._success) {
                return nil;
            }
            return painter._converted;
        }
        
        UIImage *ret = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
        if (!painter._success) {
            return nil;
        }
        return ret;
    }
    
    
    void method paint(const NamedImage &paintable) const
    {
        string imageName = paintable.imageName();
        UIImage *image = nil;
        std::string kPrefix("noart");
        if (_rect.size.width < 200 && imageName.compare(0, kPrefix.length(), kPrefix)== 0) {
            image = [UIImage imageNamed:Gear::convertString(imageName + "-small")];
        }
        if (image == nil) {
            image = [UIImage imageNamed:Gear::convertString(imageName)];
        }
        auto tintColor = paintable.tintColor();
        if (tintColor) {
            image = tint(image, tintColor);
        }
        
        CGRect r;
        if (!_stretch) {
            r = CGRectMake(_rect.origin.x + (_rect.size.width - image.size.width) /2,
                              _rect.origin.y + (_rect.size.height - image.size.height) /2,
                              (image.size.width),
                              (image.size.height));
        } else {
            r = _rect;
        }
        
        if (_callIfCantConvert) {
            _callIfCantConvert();
        }
        
        [image drawInRect:r];
    }
    
    
	static vector<pair<string, UIImage *>> _cachedBitmaps;
	static vector<pair<string, UIImage *>> _cachedBigBitmaps;
	static Base::SerialExecutor _converter;
    
	static UIImage *bitmapForImage(const weak_ptr<BitmapImage> &bitmapImage, CGSize size, vector<pair<string, UIImage *>> &cache, int limit)
	{
        string name;
        {
            auto s = bitmapImage.lock();
            if (!s) {
                return nil;
            }
            name = s->imageName();
            std::stringstream ss;
            // this seems to be wrong: the updated, bigger image would have the same fingerprint this way, so it never gets updated, really
            //ss << name << size.width << "x" << size.height;
            auto byteSize = s->data().size();
            if (byteSize == 0) {
                return nil;
            }
            ss << name << byteSize;
            name = ss.str();
        }
        
		for (auto it = cache.begin() ; it != cache.end() ; ++it) {
			if (it->first == name) {
				return [[it->second retain] autorelease];
			}
		}
        
		// not found
		while (cache.size() > limit) {
			// don't have too big cache
			auto it = cache.begin();
            [it->second release];
			cache.erase(it);
		}
        
		// add the null entry first, this will be replaced by the valid value
		UIImage *nullBitmap = nil;
		cache.push_back(std::make_pair(name, nullBitmap));
        
        _converter.addTask([bitmapImage,size,&cache,name]{
        
        // this must be serialized in order to avoid completely the exhausting the machine...
        //dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
            
            auto strongImage = bitmapImage.lock();
            if (!strongImage) {
                //assert(true);
                return;
            }
            const auto &d = strongImage->data();
            NSData *data = [NSData dataWithBytes:(void *)&d.front() length:d.size()];
            
            // debug very large files
            //static NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:@"http://upload.wikimedia.org/wikipedia/commons/a/a0/Waterfalls-picture_-_West_Virginia_-_ForestWander.jpg"]];
            //static NSData *data = [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:nil];
            UIImage *image = [UIImage imageWithData:data];
            assert(image != nil);
            
            // turns out this is not necessary anymore, and just slows us down
            // and more importantly, causes _huge_ leak, seen as 'VM: CG Raster Data' in Instruments
            /*if (image) {
                // minify image to avoid slowing down ui thread
                static CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
                CGContextRef context = CGBitmapContextCreate(NULL, size.width, size.height, 8, size.width*4, colorSpace, kCGImageAlphaNoneSkipLast);
                CGContextDrawImage(context, CGRectMake(0, 0, size.width, size.height), image.CGImage);
                CGImageRef resizedImage = CGBitmapContextCreateImage(context);
                image = [[[UIImage alloc] initWithCGImage:resizedImage] autorelease];
                CGImageRelease(resizedImage);
                CGContextRelease(context);
            }*/
                
            if (image) {
                [image retain];
                Base::MainExecutor::instance().addTask([image,name,&cache]{
					for (auto it = cache.begin() ; it != cache.end() ; ++it) {
						if (it->first == name) {
							it->second = image;
						}
					}
				});
			}
		});
        
		return nullptr;
	}
    
    UIImage * method tint(UIImage *image, const Color &tintColor)
    {
        CGSize size = [image size];
        UIGraphicsBeginImageContextWithOptions(size, NO, 0.0f);
        
        CGRect rect = CGRectZero;
        rect.size = size;
        
        // Composite tint color at its own opacity.
        UIColor *color = convertColor(tintColor);
        
        [color set];
        UIRectFill(rect);
        
        // Mask tint color-swatch to this image's opaque mask.
        // We want behaviour like NSCompositeDestinationIn on Mac OS X.
        [image drawInRect:rect blendMode:kCGBlendModeDestinationIn alpha:1.0];
        
        image = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        return image;
    }
    
    static UIImage *bitmapForImage(const shared_ptr<BitmapImage> &bitmapImage, CGSize size)
	{
        if (size.width > 256) {
            return bitmapForImage(bitmapImage, size, _cachedBigBitmaps, 1);
        } else {
            return bitmapForImage(bitmapImage, size, _cachedBitmaps, 64);
        }
    }
    
    void method paint(const BitmapImage &bitmap) const
    {
        CGSize size = _rect.size;
        // for the sake of simplicity, let's pretend everything is retina
        size.width = size.width *2;
        size.height = size.height *2;
        UIImage *decoded = bitmapForImage(MEMORY_NS::static_pointer_cast<BitmapImage>(_paintable), size);
        if (_converting) {
            _wasConverted = true;
            _converted = decoded;
            return;
        }
        
        if (_callIfCantConvert) {
            _callIfCantConvert();
        }
        
        if (decoded) {
            [decoded drawInRect:_rect];
        } else {
            _success = false;
        }
    }
    
    void method paint(const Gradient &gradient) const
    {
        if (_callIfCantConvert) {
            _callIfCantConvert();
        }
        
        CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
        
        const auto &colors = gradient.colors();
        int count = colors.size();
        
        float locations[count];
        float components[count * 4];
        
        int i = 0;
        for (auto &p : colors) {
            
            components[i * 4 + 0] = p.first.red();
            components[i * 4 + 1] = p.first.green();
            components[i * 4 + 2] = p.first.blue();
            components[i * 4 + 3] = p.first.alpha();
            
            locations[i] = p.second;
            
            ++i;
        }
        
        CGContextRef context = UIGraphicsGetCurrentContext();
        float width = CGBitmapContextGetWidth(context);
        float height = CGBitmapContextGetHeight(context);
        
        CGGradientRef cgGrad = CGGradientCreateWithColorComponents(rgb, components, locations, count);

        CGContextDrawLinearGradient(context, cgGrad, CGPointMake(width/2, height/ [[UIScreen mainScreen] scale]), CGPointMake(width/2, 0), 0);
        
        CGGradientRelease(cgGrad);
        CGColorSpaceRelease(rgb);
    }
}
