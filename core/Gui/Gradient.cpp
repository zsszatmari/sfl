//
//  Gradient.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/11/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "Gradient.h"
#include "Color.h"
#include "Gradient.h"
#include "IPainter.h"

namespace Gui
{
#define method Gradient::
    
    method Gradient(const vector<pair<Color, float>> &colors)
    {
        _colors = colors;
        if (_colors.size() == 0) {
            return;
        }
        auto f = _colors.front();
        if (f.second > 0.0f) {
            _colors.insert(_colors.begin(), std::make_pair(f.first, 0.0f));
        }
        auto b = _colors.back();
        if (b.second < 1.0f) {
            _colors.push_back(std::make_pair(b.first, 1.0f));
        }
    }
    
    method Gradient(const Color &from, const Color &to)
    {
        _colors.push_back(std::make_pair(from, 0.0f));
        _colors.push_back(std::make_pair(to, 1.0f));
    }
    
    const vector<pair<Color, float>> & method colors() const
    {
        return _colors;
    }
    
    void method paint(const IPainter &painter) const
    {
        painter.paint(*this);
    }

    static inline float interpolate(float ratio, float low, float up)
    {
    	return low + (up-low)*ratio;
    }

    Color method colorAt(float ratio) const
    {
    	float lowerRatio = 0;
    	Color lowerColor;
    	float upperRatio = 1;
    	Color upperColor;

    	for (auto it = _colors.begin() ; it != _colors.end() ; ++it) {
    		float borderRatio = it->second;
    		if (/*always true: borderRatio >= lowerRatio && */ borderRatio <= ratio) {
    			lowerRatio = borderRatio;
    			lowerColor = it->first;
    		}
    		if (borderRatio <= upperRatio && borderRatio >= ratio) {
    			upperRatio = borderRatio;
    			upperColor = it->first;
    		}
    	}
    	if (upperRatio == lowerRatio) {
    		return lowerColor;
    	}
    	ratio = (ratio-lowerRatio) / (upperRatio-lowerRatio);

    	return Color(interpolate(ratio, lowerColor.red(), upperColor.red()),
    			interpolate(ratio, lowerColor.green(), upperColor.green()),
    			interpolate(ratio, lowerColor.blue(), upperColor.blue()),
    			interpolate(ratio, lowerColor.alpha(), upperColor.alpha()) );
    }
}
