//
//  BlueTheme.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <vector>
#include "BlueTheme.h"
#include "Color.h"
#include "Gradient.h"
#include "NamedImage.h"
#include "Tinter.h"

namespace Gui
{
#define method BlueTheme::
    
    method BlueTheme()
    {
        _style = Style(R"(
        )");
    }

    shared_ptr<IPaintable> method topBar() const
    {
        Color firstColor(0.0f/255.0f, 26.0f/255.0f, 54.0f/255.0f);
        Color secondColor(5.0f/255.0f, 55.0f/255.0f, 94.0f/255.0f);
        std::vector<std::pair<Color, float>> colors;
        colors.push_back(std::make_pair(firstColor, 0.0f));
        colors.push_back(std::make_pair(secondColor, 0.76f));
        
        return shared_ptr<Gradient>(new Gradient(colors));
    }
    
    shared_ptr<IPaintable> method bottomBar() const
    {
        Color firstColor(0.0f/255.0f, 26.0f/255.0f, 54.0f/255.0f);
        Color secondColor(5.0f/255.0f, 55.0f/255.0f, 94.0f/255.0f);
        std::vector<std::pair<Color, float>> colors;
        colors.push_back(std::make_pair(secondColor, 0.24f));
        colors.push_back(std::make_pair(firstColor, 1.0f));
        
        return shared_ptr<Gradient>(new Gradient(colors));
    }
    
    Color method textHighlightColor() const
    {
        return Color(78.0f/255.0f, 233.0f/255.0f, 225.0f/255.0f);
    }
    
    Color method textColor() const
    {
        return Color(1.0f, 1.0f, 1.0f);
    }

	Color method textHeaderColor() const
	{
		return Color(1.0f, 1.0f, 1.0f);
	}
    
    shared_ptr<IPaintable> method settingsIcon()
    {
        return shared_ptr<IPaintable>(new NamedImage("category-settings"));
    }
    
    shared_ptr<IPaintable> method listBackground() const
    {
        //return shared_ptr<Color>(new Color(0.34f, 0.0f/255.f, 0.0f/255.0f));
        return shared_ptr<Color>(new Color(0.0f, 26.0f/255.f, 54.0f/255.0f));
    }
    
    shared_ptr<IPaintable> method listSeparator() const
    {
        return shared_ptr<Color>(new Color(125.0f/255.0f, 125.0f/255.0f, 125.0f/255.0f, 0.0f));
        //return shared_ptr<Color>(new Color(125.0f/255.0f, 125.0f/255.0f, 125.0f/255.0f));
    }
    
    float method listSeparatorThickness() const
    {
        return 1;
    }
    
    TextAttributes method categoryText() const
    {
        return TextAttributes(TextAttributes::Font::Title);
    }
    
    TextAttributes method navigationText() const
    {
        return TextAttributes(TextAttributes::Font::Category, Color(1,1,1));
    }
    
    TextAttributes method listTitleText(bool current) const
    {
        return TextAttributes(TextAttributes::Font::Title, current ? textHighlightColor() : Color(1,1,1));
    }
    
    TextAttributes method listSubtitleText(bool current) const
    {
		return TextAttributes(TextAttributes::Font::Subtitle, current ? textHighlightColor() : Color(1, 1, 1));
    }
    
    TextAttributes method listSubSubtitleText(bool current) const
    {
    	// bottom bar selectedColor: (78.0f/255.0f, 233.0f/255.0f, 225.0f/255.0f);
        return TextAttributes(TextAttributes::Font::Subtitle,
        		current ? Color(78.0f/255.0f*109.0f/255.0f, 233.0f/255.0f*134.0f/255.0f, 225.0f/255.0f*136.0f/255.0f) : Color(109.0f/255.0f, 134.0f/255.0f, 136.0f/255.0f));
    }
    
    shared_ptr<IPaintable> method playIcon() const
    {
        return shared_ptr<IPaintable>(new NamedImage("play"));
    }
    
    shared_ptr<IPaintable> method pauseIcon() const
    {
        return Tinter::tintWithColor(shared_ptr<IPaintable>(new NamedImage("pause")), Color(1.0f,1.0f,1.0f,1.0f));
    }
    
    shared_ptr<IPaintable> method prevIcon() const
    {
        return shared_ptr<IPaintable>(new NamedImage("prev"));
    }
    
    shared_ptr<IPaintable> method nextIcon() const
    {
        return shared_ptr<IPaintable>(new NamedImage("next"));
    }
    
    TextAttributes method miniplayerTitleText() const
    {
        return TextAttributes(TextAttributes::Font::Playing, Color(1,1,1));
    }
    
    TextAttributes method miniplayerSubtitleText() const
    {
        return TextAttributes(TextAttributes::Font::PlayingSub, Color(1,1,1));
    }
    
    TextAttributes method miniplayerTimeText() const
    {
        return TextAttributes(TextAttributes::Font::Footnote, Color(1,1,1));
    }
    
    Color method gray() const
    {
        return Color(120.0f/255.0f, 134.0f/255.0f, 160.0f/255.0f);
    }
    
    Color method selected() const
    {
        return Color(0.0f/255.0f, 174.0f/255.0f, 232.0f/255.0f);
    }
    
    Color method activeColor(bool isSelected) const
    {
        return isSelected ? selected() : gray();
    }
    
    shared_ptr<IPaintable> method shuffleIcon(bool selected) const
    {
        return Tinter::tintWithColor(shared_ptr<IPaintable>(new NamedImage("shuffle")), activeColor(selected));
    }
    
    shared_ptr<IPaintable> method repeatIcon(bool selected) const
    {
        return Tinter::tintWithColor(shared_ptr<IPaintable>(new NamedImage("repeat")), activeColor(selected));
    }
    
    shared_ptr<IPaintable> method shareIcon(bool selected) const
    {
        return Tinter::tintWithColor(shared_ptr<IPaintable>(new NamedImage("share")), activeColor(selected));
    }
    
    BlueTheme::SliderLook method seekSlider() const
    {
        SliderLook look;
        shared_ptr<IPaintable> center(new Gui::Color(143.0f/255.0f, 188.0f/255.0f, 187.0f/255.0f));
        look.thumb = center;
        look.thickness = 3.5f;
        //look.leftSideWidth = 1.0f;
        //look.rightSideWidth = 0.0f;
        look.leftSide = center;
        look.thumbWidth = 1.0f;
        
        using std::make_pair;
        {
            Gui::Color edge(151.0f/255.0f, 186.0f/255.0f, 186.0f/255.0f);
            Gui::Color center(1.0f, 1.0f, 1.0f);
            vector<pair<Gui::Color, float>> colors;
            colors.push_back(make_pair(edge, 0.0f));
            colors.push_back(make_pair(edge, 2.0f/7.0f));
            colors.push_back(make_pair(center, 2.0f/7.0f));
            colors.push_back(make_pair(center, 5.0f/7.0f));
            colors.push_back(make_pair(edge, 5.0f/7.0f));
            colors.push_back(make_pair(edge, 1.0f));
            shared_ptr<IPaintable> rightGradient(new Gui::Gradient(colors));
            look.minimumTrack = rightGradient;
        }
        
        {
            Gui::Color rightTop(2.0f/255.0f, 0.0f/255.0f, 34.0f/255.0f);

            shared_ptr<IPaintable> rightGradient(new Gui::Color(2.0f/255.0f, 0.0f/255.0f, 34.0f/255.0f));
            look.maximumTrack = rightGradient;
        }
        
        return look;
    }

	shared_ptr<IPaintable> method playlistListBackground()
	{
		return shared_ptr<Color>(new Color(5.0f / 255.0f, 55.0f / 255.0f, 94.0f / 255.0f));
	}

	shared_ptr<IPaintable> method playlistHighlightBackground()
	{
		return shared_ptr<Color>(new Color(10.0f / 255.0f, 75.0f / 255.0f, 104.0f / 255.0f));
	}

	shared_ptr<IPaintable> method playlistBackground()
	{
		return shared_ptr<Color>(new Color(0.0f / 255.0f, 45.0f / 255.0f, 84.0f / 255.0f));
	}

	Color method songOddBackground()
	{
		return Color(0.0f / 255.0f, 55.0f / 255.0f, 95.0f / 255.0f);
	}

	Color method songEvenBackground()
	{
		return Color(0.0f / 255.0f, 45.0f / 255.0f, 84.0f / 255.0f);
	}
    
    std::string method name() const
    {
        return "Blue";
    }

    string method noArtName() const
    {
        return "noart-blue";
    }

}
