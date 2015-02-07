//
//  ThemeManager.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "ThemeManager.h"
#include "BlueTheme.h"
#include "BlackTheme.h"
#include "ModernDarkTheme.h"
#include "IApp.h"
#include "IPreferences.h"
#include "sfl/Vector.h"
#include "sfl/Prelude.h"

namespace Gui
{
    using namespace sfl;
    
#define method ThemeManager::
    
    method ThemeManager() :
#if TARGET_OS_IPHONE || _WIN32
        _current(nullptr)
#else
    // for now...
        _current(new ModernDarkTheme())
#endif
    {
    }

    const Style & method style() const
    {
        return _current->style();
    }    
    
    shared_ptr<ITheme> method current() const
    {
        if (!_current) {
            auto t = themes();

            auto key = Gear::IApp::instance()->preferences().stringForKey("Theme");
            _current = snd(maybe(t.at(0),find([&](const std::pair<std::string,shared_ptr<ITheme>> &p){return p.first == key;}, t)));
        }
        return _current;
    }

    void method refresh()
    {
        _current = nullptr;
        Gear::IApp::instance()->applyTheme();
    }
    
    void method setCurrent(const shared_ptr<ITheme> &theme)
    {
        _current = theme;
    }

     std::vector<std::pair<std::string,shared_ptr<ITheme>>> method themes()
    {
        // by convention, first one is the default

        std::vector<std::pair<std::string,shared_ptr<ITheme>>> ret;
        ret.push_back(std::make_pair("Blue",shared_ptr<ITheme>(new BlueTheme())));
        ret.push_back(std::make_pair("Black",shared_ptr<ITheme>(new BlackTheme())));
        return ret;
    }
}
