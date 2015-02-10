//
//  BitmapImage.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/31/13.
//
//

#ifndef __G_Ear_Player__BitmapImage__
#define __G_Ear_Player__BitmapImage__

#include <string>
#include <vector>
#include "Environment.h"
#include "IPaintable.h"

namespace Gui
{
    using std::vector;
    
    class IPainter;
    
    class core_export BitmapImage final : public IPaintable
    {
    public:

        enum class Format
        {
            Auto,
            Rgba
        };

        BitmapImage(const vector<char> &data, const std::string &name);
        BitmapImage(const vector<char> &data, Format format, size_t width, size_t height);

        virtual void paint(const IPainter &painter) const;
        const vector<char> &data() const;
        const std::string &imageName() const;
        Format format() const;
        size_t width() const;
        size_t height() const;
        
    private:
        Format _format;
        vector<char> _data;
        std::string _name;
        size_t _width;
        size_t _height;
    };
}

#endif /* defined(__G_Ear_Player__BitmapImage__) */
