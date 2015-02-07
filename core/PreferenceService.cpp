//
//  PreferenceService.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 02/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "IApp.h"
#include "PreferenceService.h"
#include "IPreferences.h"
#include "PreferencesPanel.h"

namespace Gear
{
#define method PreferenceService::
    
    void method autoconnect()
	{
		if (IApp::instance()->preferences().boolForKey(preferencesKey()) == enabledByDefault()) {
			_state = IService::State::Offline;
		} else {
            PreferencesPanel::checkService(*this, [&]{
                _state = IService::State::Online;
                connected();
            }, []{
                //std::cout << "nooo\n";
            });
		}
	}
    
	void method connect()
	{
		IApp::instance()->preferences().setUintForKey(preferencesKey(), !enabledByDefault());
		_state = IService::State::Online;
		connected();
	}
    
	void method disconnect()
	{
		IApp::instance()->preferences().setUintForKey(preferencesKey(), enabledByDefault());
		_state = IService::State::Offline;
        disconnected();
	}
    
    bool method enabledByDefault()
    {
        return false;
    }
}