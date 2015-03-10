//
//  ISession.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#include "ISession.h"
#include "SongEntry.h"
#include "PlaylistCategory.h"
#include "IApp.h"
#include "IPreferences.h"

namespace Gear
{
    const char kAllCategory[] = "All";
    const char kPlaylistsCategory[] = "Playlists";
    const char kAutoPlaylistsCategory[] = "Auto Playlists";
    const char kSubscriptionsCategory[] = "Subscriptions";

    
#define method ISession::
    
    method ISession(const shared_ptr<IApp> &app) :
        _app(app),
        _refreshing(false)
    {
    }
    
    method ~ISession()
    {
    }
    
    const shared_ptr<IApp> method app()
    {
        return _app;
    }
    
    EventConnector method connectedEvent()
    {
        return _connectedSignal.connector();
    }
    
    EventConnector method playlistsChangeEvent()
    {
        return _playlistsChangeSignal.connector();
    }

    PlaylistCategory method categoryByTag(int tag)
    {
    	auto cat = categories();
    	for (auto it = cat->begin() ; it != cat->end() ; ++it) {
    		auto &category = *it;
    		if (category.tag() == tag) {
                return category;
            }
        }
        static PlaylistCategory notFound;
        return notFound;
    }
    
    ValueConnector<bool> method refreshingConnector()
    {
        return _refreshing.connector();
    }

    bool method manipulationEnabled() const
	{
    	return true;
	}

    vector<int> method possibleRatings() const
	{
    	vector<int> ret;
        if (IApp::instance()->preferences().boolForKey("FiveStarRatings")) {
            ret.push_back(0);
            ret.push_back(1);
            ret.push_back(2);
            ret.push_back(3);
            ret.push_back(4);
            ret.push_back(5);
        } else {
        	ret.push_back(0);
        	ret.push_back(1);
        	ret.push_back(5);
        }
    	return ret;
	}
    
    void method freeUpMemory()
    {
    }
    
    void method setOffline(bool offline)
    {
    }
    
    void method dispose()
    {
    }

    std::string method sessionIdentifier() const
    {
        return "";
    }
    
    bool method saveForOfflinePossible() const
    {
        return false;
    }
}
