//
//  ThemeManager.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/29/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__ThemeManager__
#define __G_Ear_core__ThemeManager__

#include "stdplus.h"
#include <vector>
#include MEMORY_H

using MEMORY_NS::shared_ptr;

namespace Gui
{
    class ITheme;
    class Style;
    
    class core_export ThemeManager final
    {
    public:
        ThemeManager();
        
        void refresh();
        shared_ptr<ITheme> current() const;
        void setCurrent(const shared_ptr<ITheme> &theme);
        const Style & style() const;
        static std::vector<std::pair<std::string,shared_ptr<ITheme>>> themes();
        
    private:
        mutable shared_ptr<ITheme> _current;
    };
}

#endif /* defined(__G_Ear_core__ThemeManager__) */
