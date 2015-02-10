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
	};
}

#endif