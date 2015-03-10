#ifndef BLACKTHEME_H
#define BLACKTHEME_H

#include "BlueTheme.h"

namespace Gui
{
	class BlackTheme : public BlueTheme
	{
	public:
		virtual shared_ptr<IPaintable> topBar() const;
    	virtual shared_ptr<IPaintable> bottomBar() const;
    	virtual shared_ptr<IPaintable> listBackground() const;
    	
    	ITheme::SliderLook seekSlider() const;

		virtual std::string name() const;
	    virtual std::string noArtName() const override;

	    virtual Color selected() const override;
	    virtual Color textHighlightColor() const override;
	    virtual TextAttributes listSubSubtitleText(bool current) const override;

	    virtual Gui::Color artBackground() const override;
	    virtual Gui::Color overlayBackground() const override;
	    
		virtual Gui::Color selectedTextBackground() const override;
	};
}

#endif