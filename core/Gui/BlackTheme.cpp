#include "BlackTheme.h"
#include "Gradient.h"

namespace Gui
{
#define method BlackTheme::

    shared_ptr<IPaintable> method topBar() const
    {
        Color firstColor(27.0f/255.0f, 29.0f/255.f, 28.0f/255.0f);
        Color secondColor(52.0f/255.0f, 50.0f/255.0f, 51.0f/255.0f);
        std::vector<std::pair<Color, float>> colors;
        colors.push_back(std::make_pair(firstColor, 0.0f));
        colors.push_back(std::make_pair(secondColor, 0.76f));
        
        return shared_ptr<Gradient>(new Gradient(colors));
    }
    
    shared_ptr<IPaintable> method bottomBar() const
    {
        Color firstColor(27.0f/255.0f, 29.0f/255.f, 28.0f/255.0f);
        Color secondColor(52.0f/255.0f, 50.0f/255.0f, 51.0f/255.0f);
        std::vector<std::pair<Color, float>> colors;
        colors.push_back(std::make_pair(secondColor, 0.24f));
        colors.push_back(std::make_pair(firstColor, 1.0f));
        
        return shared_ptr<Gradient>(new Gradient(colors));
    }
    
    shared_ptr<IPaintable> method listBackground() const
    {
        return shared_ptr<Color>(new Color(27.0f/255.0f, 29.0f/255.f, 28.0f/255.0f));
    }

    static shared_ptr<IPaintable> gradient(const Gui::Color &bottom, const Gui::Color &center, const Gui::Color &top)
    {
        using std::make_pair;
     
        vector<pair<Gui::Color, float>> colors;
        colors.push_back(make_pair(top, 0.0f));
        colors.push_back(make_pair(top, 2.0f/7.0f));
        colors.push_back(make_pair(center, 2.0f/7.0f));
        colors.push_back(make_pair(center, 5.0f/7.0f));
        colors.push_back(make_pair(bottom, 5.0f/7.0f));
        colors.push_back(make_pair(bottom, 1.0f));
        shared_ptr<IPaintable> grad(new Gui::Gradient(colors));
        return grad;
    }

    ITheme::SliderLook method seekSlider() const
    {
        SliderLook look;
        shared_ptr<IPaintable> center(new Gui::Color(167.0f/255.0f, 184.0f/255.0f, 185.0f/255.0f,0.0f));
        look.thumb = center;
        look.thickness = 3.5f;
        //look.leftSideWidth = 1.0f;
        //look.rightSideWidth = 0.0f;
        look.thumbWidth = 1.0f;
        
        look.minimumTrack = gradient(Gui::Color(131.0f/255.0f,130.0f/255.0f,130.0f/255.0f),
                                     Gui::Color(112.0f/255.0f,110.0f/255.0f,111.0f/255.0f),
                                     Gui::Color(99.0f/255.0f,99.0f/255.0f,99.0f/255.0f));
        look.maximumTrack = gradient(Gui::Color(215.0f/255.0f,215.0f/255.0f,215.0f/255.0f),
                                     Gui::Color(255.0f/255.0f,255.0f/255.0f,255.0f/255.0f),
                                     Gui::Color(95.0f/255.0f,95.0f/255.0f,95.0f/255.0f));
        look.leftSide = look.minimumTrack;
        look.rightSide = look.maximumTrack;
        

        return look;
    }

	std::string method name() const
    {
        return "Black";
    }

    std::string method noArtName() const
    {
        return "noart";
    }
}