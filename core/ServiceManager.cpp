//
//  ServiceManager.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#include "ServiceManager.h"
#include "GearUtility.h"
#include "GooglePlayService.h"
#include "YouTubeService.h"
#include "YouTubeSearchService.h"
#include "OfflineService.h"
#include "SpotifyService.h"
#include "CoreDebug.h"
#include "LocalService.h"

namespace Gear
{
#define method ServiceManager::
    
    method ServiceManager(shared_ptr<SessionManager> &sessionManager) :
        _sessionManager(sessionManager)
    {
    	shared_ptr<IService> local = LocalService::create(sessionManager);
    	if (local) {
    		_services.push_back(local);
    	}
        shared_ptr<IService> goo = shared_ptr<GooglePlayService>(new GooglePlayService(sessionManager));
#ifndef DISABLE_YOUTUBE
        // for now, only testing on mac in-house
        shared_ptr<IService> utubeSearch = YouTubeSearchService::create(sessionManager);
        shared_ptr<IService> utube = YouTubeService::create(sessionManager);
        _services.push_back(utubeSearch);
        _services.push_back(utube);
#endif
        _services.push_back(goo);
#ifndef DISABLE_YOUTUBE
        _services.push_back(shared_ptr<OfflineService>(new OfflineService()));
#endif
#ifndef DISABLE_SPOTIFY
        _services.push_back(SpotifyService::create(sessionManager));
#endif
    }

    IService::State method generalState() const
    {
    	for (auto it = _services.begin() ; it != _services.end() ; ++it) {
    		auto &service = *it;
    		auto state = service->state();
    		if (state != IService::State::Offline) {
    			return state;
    		}
    	}
		return IService::State::Offline;
    }
    
    vector<shared_ptr<IService>> method services() const
    {
        return _services;
    }
    
    void method autoconnect()
    {
#ifdef DEBUG_DISABLE_AUTOCONNECT
        return;
#endif
        for (auto it = _services.begin() ; it != _services.end() ; ++it) {
        	auto &s = *it;
        	s->autoconnect();
        }
    }
    
    const shared_ptr<IService> method googlePlayService() const
    {
        for (auto it = _services.begin() ; it != _services.end() ; ++it) {
            auto &service = *it;
            if (MEMORY_NS::dynamic_pointer_cast<GooglePlayService>(service)) {
                return service;
            }
        }
        return shared_ptr<IService>();
    }
}
