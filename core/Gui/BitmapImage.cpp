//
//  BitmapImage.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#include "BitmapImage.h"
#include "IPainter.h"

namespace Gui
{
#define method BitmapImage::
    
    method BitmapImage(const vector<char> &data, const std::string &name) :
        _data(data),
        _name(name),
        _format(Format::Auto),
        _width(0),
        _height(0)
    {
    }

    method BitmapImage(const vector<char> &data, Format format, size_t width, size_t height) :
        _data(data),
        _format(format),
        _width(width),
        _height(height)
    {
    }
    
    void method paint(const IPainter &painter) const
    {
        painter.paint(*this);
    }
    
    const vector<char> & method data() const
    {
        return _data;
    }
    
    const std::string & method imageName() const
    {
        return _name;
    }


    BitmapImage::Format method format() const
    {
        return _format;
    }

    size_t method width() const
    {
        return _width;
    }

    size_t method height() const
    {
        return _height;
    }
}
