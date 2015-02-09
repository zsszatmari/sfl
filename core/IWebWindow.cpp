//
//  IWebWindow.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/4/13.
//
//

#include "IWebWindow.h"
#include "Timer.h"

namespace Gear
{
#define method IWebWindow::
    
    method IWebWindow(const shared_ptr<IWebWindowDelegate> &delegate) :
        _delegate(delegate)
    {
    }
    
    method ~IWebWindow()
    {
    }

    void method checkPeriodically(const shared_ptr<IWebWindow> &webWindow)
    {
    	MEMORY_NS::weak_ptr<IWebWindow> w = webWindow;
    	webWindow->_timer = shared_ptr<Base::Timer>(new Base::Timer([w](Base::Timer *){
    		auto s = w.lock();
    		if (s) {
    			if (s->_delegate) {
    				s->_delegate->didFinishLoad();
    			}
    		}
    	}, 1, true, 0.05));
    }

}