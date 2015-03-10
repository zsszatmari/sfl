//
//  Painter.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#include "Painter.h"
#include "IPaintable.h"
#include "BitmapImage.h"
#include "NamedImage.h"
#include "StringUtility.h"
#include "Gradient.h"
#import "NSImage+Tinting.h"


namespace Gui
{
    class ImagePainter final : public IPainter
    {
    public:
        virtual void paint(const Color &color) const
        {
        }
        
        virtual void paint(const NamedImage &paintable) const
        {
            NSString *name = Gear::convertString(paintable.imageName());
            NSImage *image = [NSImage imageNamed:name];
            auto tintColor = paintable.tintColor();
            if (tintColor) {
                image = [image imageTintedWithColor:Painter::convertColor(tintColor)];
            }
            
            _image = image;
            /*
            CGRect r;
            if (!_stretch) {
                r = CGRectMake(_rect.origin.x + (_rect.size.width - image.size.width) /2,
                               _rect.origin.y + (_rect.size.height - image.size.height) /2,
                               (image.size.width),
                               (image.size.height));
            } else {
                r = _rect;
            }
            [image drawInRect:r];*/
        }
        
        virtual void paint(const BitmapImage &image) const
        {
            auto &v = image.data();
            if (v.empty()) {
                return;
            }
            //NSData *data = [[NSData alloc] initWithBytesNoCopy:(void *)&v.front() length:v.size() freeWhenDone:NO];
            NSData *data = [[NSData alloc] initWithBytes:(void *)&v.front() length:v.size()];
            
            _image = [[NSImage alloc] initWithData:data];
        }
        
        virtual void paint(const Gradient &gradient) const
        {
        }
        
    private:
        mutable NSImage *_image;
        
    friend class Painter;
    };
    
#define method Painter::
    
    method Painter(CGRect frame) :
        _frame(frame)
    {
    }
    
    void method paint(const IPaintable &paintable, CGRect rect)
    {
        Painter painter(rect);
        paintable.paint(painter);
    }
    
    void method paint(const Color &color) const
    {
        [convertColor(color) setFill];
        NSRectFill(_frame);
    }
    
    void method paint(const NamedImage &paintable) const
    {
        NSImage *image = [NSImage imageNamed:Gear::convertString(paintable.imageName())];
        [image drawInRect:_frame fromRect:CGRectMake(0,0,image.size.width,image.size.height) operation:NSCompositeSourceOver fraction:1.0f respectFlipped:YES hints:nil];
    }
    
    void method paint(const BitmapImage &image) const
    {
        if (image.format() == BitmapImage::Format::Rgba) {
            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
            CGContextRef bitmapContext = CGBitmapContextCreate(
                                                               (void *)(image.data().data()),
                                                               image.width(),
                                                               image.height(),
                                                               8, // bitsPerComponent
                                                               4*image.width(), // bytesPerRow
                                                               colorSpace,
                                                               kCGImageAlphaPremultipliedLast);
            CGImageRef cgImage = CGBitmapContextCreateImage(bitmapContext);
            NSImage *nsImage = [[NSImage alloc] initWithCGImage:cgImage size:CGSizeMake(image.width(), image.height())];
            
            CGImageRelease(cgImage);
            CGContextRelease(bitmapContext);
            CGColorSpaceRelease(colorSpace);
            
            [nsImage drawInRect:_frame];
        }
    }
    
    void method paint(const Gradient &gradient) const
    {
        NSMutableArray *colors = [[NSMutableArray alloc] init];
        std::vector<CGFloat> locations;
        
        for (auto &p : gradient.colors()) {
            locations.push_back(p.second);
            [colors addObject:convertColor(p.first)];
        }
        
        NSGradient *grad = [[NSGradient alloc] initWithColors:colors atLocations:&locations.front() colorSpace:[NSColorSpace deviceRGBColorSpace]];
        [grad drawInRect:_frame angle:270];
    }
    
    NSImage * method convertImage(const IPaintable &paintable)
    {
        ImagePainter painter;
        
        paintable.paint(painter);
        return painter._image;
    }
    
    NSColor * method convertColor(const Color &color)
    {
        return [NSColor colorWithDeviceRed:color.red() green:color.green() blue:color.blue() alpha:color.alpha()];
    }
}