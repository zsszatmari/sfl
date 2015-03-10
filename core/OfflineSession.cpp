//
//  OfflineSession.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmari on 01/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#include "OfflineSession.h"
#include "PlaylistCategory.h"
#include "OfflineStorage.h"
#include "NamedImage.h"
#include "OfflineService.h"
#include "ISong.h"

namespace Gear
{
#define method OfflineSession::
    
    /*shared_ptr<OfflineSession> method create(const shared_ptr<IApp> &app)
    {
        auto ret = shared_ptr<OfflineSession>(new OfflineSession(app));
        ret->createCategories();
        return ret;
    }*/
    
    vector<int> method possibleRatings() const
    {
        vector<int> ret;
    	ret.push_back(0);
    	return ret;
    }
    
    method OfflineSession(const shared_ptr<IApp> &app) :
        ISession(app),
        _offline(false)
    {
        _updateCounter = 0;
    }
    
    /*
    shared_ptr<IPlaylist> method libraryPlaylist()
    {
        return _libraryPlaylist;
    }*/
    
    void method refresh()
    {
    }
    
    void method addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs)
    {
    }
    
    shared_ptr<IPlaybackData> method playbackDataSync(const ISong &song) const
    {
        const string uniqueId = song.uniqueId();
        if (OfflineStorage::instance().available(song)) {
            static const string youtube("youtube");
            return OfflineStorage::instance().fetch(uniqueId, song.stringForKey("sourceService") == youtube ? IPlaybackData::Format::Mp4 : IPlaybackData::Format::Mp3);
        }
        return nullptr;
    }
    
    ValidPtr<const vector<PlaylistCategory>> method categories() const
    {
        if (!OfflineService::available() || OfflineService::disabledStatic()) {
            return ValidPtr<const vector<PlaylistCategory>>();
        }
        
        //return _fullCategories;
        if (_offline) {
            return _fullCategories;
        } else {
            return _categories;
        }
    }
    
    void method setOffline(bool offline)
    {
        _offline = offline;
    }
    
    std::string method sessionIdentifier() const
    {
        return "offline";
    }
    
    bool method saveForOfflinePossible() const
    {
        // actually, this is for remove from offline
        return true;
    }
}