//
//  IWebWindowDelegate.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/4/13.
//
//

#ifndef __G_Ear_Player__IWebWindowDelegate__
#define __G_Ear_Player__IWebWindowDelegate__

#include <string>
#include <functional>
#include "stdplus.h"
#include MEMORY_H

namespace Gear
{
    using std::string;
    class IWebWindow;
    
    class IWebWindowDelegate
    {
    public:
        virtual ~IWebWindowDelegate();
        virtual void didFailLoad(const string &errorMessage) = 0;
        virtual void didFinishLoad() = 0;
        virtual void navigationAction() = 0;
        virtual bool controlledCookies() const;
        // this lets us monitor web activity to a lesser extents, however it is more convenient for the user. used for last.fm
        virtual bool allowStandaloneBrowser() const;

        // must be called from main thread
        // this is a more functional variant of the delegate
        static MEMORY_NS::shared_ptr<IWebWindow> create(const std::function<void(const MEMORY_NS::shared_ptr<IWebWindow> &, const std::function<void()> &dispose)> &finish, const std::function<void(const MEMORY_NS::shared_ptr<IWebWindow> &w, const std::function<void()> &dispose)> &fail, const std::function<void(const MEMORY_NS::shared_ptr<IWebWindow> &w, const std::function<void()> &dispose)> &navigation);
    };
}

#endif /* defined(__G_Ear_Player__IWebWindowDelegate__) */
