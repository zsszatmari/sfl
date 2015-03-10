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

    Gui::Color method artBackground() const
    {
        return Gui::Color(0,0,0);
    }

    Gui::Color method overlayBackground() const
    {
        return Gui::Color(0,0,0,0.7f);
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
        //shared_ptr<IPaintable> center(new Gui::Color(167.0f/255.0f, 184.0f/255.0f, 185.0f/255.0f,0.0f));
        //look.thumb = center;
        look.thickness = 3.5f;
        //look.leftSideWidth = 1.0f;
        //look.rightSideWidth = 0.0f;
        look.thumbWidth = 1.0f;
        
        look.maximumTrack = shared_ptr<Gui::Color>(new Color(87.0f/255.0f,89.0f/255.0f,88.0f/255.0f));
        look.minimumTrack = gradient(Gui::Color(215.0f/255.0f,215.0f/255.0f,215.0f/255.0f),
                                     Gui::Color(255.0f/255.0f,255.0f/255.0f,255.0f/255.0f),
                                     Gui::Color(215.0f/255.0f,215.0f/255.0f,215.0f/255.0f));
        
        shared_ptr<Gui::Color> center(new Gui::Color(215.0f/255.0f,215.0f/255.0f,215.0f/255.0f));
        look.thumb = center;
        look.leftSide = center;
        
        look.rightSide = look.maximumTrack;
        

        return look;
    }

	std::string method name() const
    {
        return "Black";
    }

    std::string method noArtName() const
    {
        return "noart-black";
    }

    Color method textHighlightColor() const
    {
        return selected();
    }

    Color method selected() const
    {
        return Color(0.0f/255.0f, 162.0f/255.0f, 255.0f/255.0f);
    }

    TextAttributes method listSubSubtitleText(bool current) const
    {
        // bottom bar selectedColor: (78.0f/255.0f, 233.0f/255.0f, 225.0f/255.0f);
        return TextAttributes(TextAttributes::Font::Subtitle,
                current ? Color(0.0f/255.0f*109.0f/255.0f, 162.0f/255.0f*134.0f/255.0f, 255.0f/255.0f*136.0f/255.0f) : Color(109.0f/255.0f, 134.0f/255.0f, 136.0f/255.0f));
    }

    Gui::Color method selectedTextBackground() const
    {
        return Gui::Color(17.0f / 255.0f, 71.0f/ 255.0f, 120.0f / 255.0f, 0.7f);
    }
}