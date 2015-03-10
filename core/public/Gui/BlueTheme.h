//
//  BlueTheme.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__BlueTheme__
#define __G_Ear_core__BlueTheme__

#include "ITheme.h"

namespace Gui
{
    class BlueTheme : public ITheme
    {
    public:
        BlueTheme();
        virtual shared_ptr<IPaintable> topBar() const;
        virtual shared_ptr<IPaintable> bottomBar() const;
        virtual Color textHighlightColor() const;
        virtual Color textColor() const;
		virtual Color textHeaderColor() const;

		virtual shared_ptr<IPaintable> playlistListBackground();
		virtual shared_ptr<IPaintable> playlistHighlightBackground();
		virtual shared_ptr<IPaintable> playlistBackground();

		virtual Color songOddBackground();
		virtual Color songEvenBackground();

        virtual shared_ptr<IPaintable> settingsIcon();
        
        virtual shared_ptr<IPaintable> listBackground() const;
        virtual shared_ptr<IPaintable> listSeparator() const;
        virtual float listSeparatorThickness() const;
        
        virtual TextAttributes categoryText() const;
        virtual TextAttributes navigationText() const;
        virtual TextAttributes listTitleText(bool current = false) const;
        virtual TextAttributes listSubtitleText(bool current = false) const;
        virtual TextAttributes listSubSubtitleText(bool current = false) const;
        
        virtual TextAttributes miniplayerTitleText() const;
        virtual TextAttributes miniplayerSubtitleText() const;
        virtual TextAttributes miniplayerTimeText() const;
        
        virtual shared_ptr<IPaintable> playIcon() const;
        virtual shared_ptr<IPaintable> pauseIcon() const;
        virtual shared_ptr<IPaintable> prevIcon() const;
        virtual shared_ptr<IPaintable> nextIcon() const;
        virtual shared_ptr<IPaintable> shuffleIcon(bool active) const;
        virtual shared_ptr<IPaintable> repeatIcon(bool active) const;
        virtual shared_ptr<IPaintable> shareIcon(bool active) const;
        
        virtual SliderLook seekSlider() const;
        virtual std::string name() const;
        virtual std::string noArtName() const override;
        
    private:
        Color activeColor(bool isSelected) const;
        virtual Color selected() const;
        virtual Color gray() const;
        
    };
}

#endif /* defined(__G_Ear_core__BlueTheme__) */
