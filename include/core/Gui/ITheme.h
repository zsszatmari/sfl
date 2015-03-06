//
//  ITheme.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ITheme__
#define __G_Ear_core__ITheme__

#include <string>
#include "stdplus.h"
#include "Color.h"
#include "TextAttributes.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;
#include "Style.h"

namespace Gui
{
    class IPaintable;
    
    class core_export ITheme
    {
    public:
		virtual shared_ptr<IPaintable> topBar() const = 0;
		virtual shared_ptr<IPaintable> bottomBar() const = 0;

		virtual shared_ptr<IPaintable> playIcon() const = 0;
		virtual shared_ptr<IPaintable> pauseIcon() const = 0;
		virtual shared_ptr<IPaintable> prevIcon() const = 0;
		virtual shared_ptr<IPaintable> nextIcon() const = 0;

		struct SliderLook
		{
		public:
			SliderLook() :
				sideLength(1.0f)
			{	
			}

			shared_ptr<IPaintable> minimumTrack;
			shared_ptr<IPaintable> maximumTrack;
			shared_ptr<IPaintable> leftSide;
			shared_ptr<IPaintable> rightSide;
			shared_ptr<IPaintable> thumb;
			float thickness;
			float thumbWidth;
			float sideLength;
		};
		virtual SliderLook seekSlider() const = 0;
		virtual SliderLook volumeSlider() const;

		virtual shared_ptr<IPaintable> playlistListBackground() = 0;
		virtual shared_ptr<IPaintable> playlistHighlightBackground() = 0;
		virtual shared_ptr<IPaintable> playlistBackground() = 0;

		virtual Color songOddBackground() = 0;
		virtual Color songEvenBackground() = 0;

		// these are already tinted, caller shouldn't tint it!
		virtual shared_ptr<IPaintable> ratingIcon(int rating) const;
		virtual shared_ptr<IPaintable> shuffleIcon(bool active) const = 0;
		virtual shared_ptr<IPaintable> repeatIcon(bool active) const = 0;
		virtual shared_ptr<IPaintable> shareIcon(bool active) const = 0;
		virtual shared_ptr<IPaintable> eqIcon(bool active) const;

		virtual Color textHighlightColor() const = 0;
		virtual Color textColor() const = 0;
		virtual Color textHeaderColor() const = 0;

		static shared_ptr<IPaintable> noArt();
		virtual std::string noArtName() const;

		// ---- ios legacy:
        
        virtual shared_ptr<IPaintable> settingsIcon() = 0;
        
        virtual shared_ptr<IPaintable> listBackground() const = 0;
        virtual Gui::Color artBackground() const;
	    virtual Gui::Color overlayBackground() const;
        
        virtual shared_ptr<IPaintable> listSeparator() const = 0;
        virtual float listSeparatorThickness() const = 0;
        
        virtual TextAttributes categoryText() const = 0;
        virtual TextAttributes navigationText() const = 0;
        virtual TextAttributes navigationTextActive() const;
        virtual TextAttributes listTitleText(bool current = false) const = 0;
        virtual TextAttributes listSubtitleText(bool current = false) const = 0;
        virtual TextAttributes listSubSubtitleText(bool current = false) const = 0;
        
        virtual TextAttributes miniplayerTitleText() const = 0;
        virtual TextAttributes miniplayerSubtitleText() const = 0;
        virtual TextAttributes miniplayerSubSubtitleText() const;
        virtual TextAttributes miniplayerTimeText() const = 0;
        
        virtual std::string name() const = 0;

        virtual ~ITheme();
        
        virtual Color selected() const = 0;
        virtual Color gray() const = 0;

        const Style & style() const;

		virtual Gui::Color selectedTextBackground() const;

    protected:
    	Style _style;
    };
}

#endif /* defined(__G_Ear_core__ITheme__) */
