//
//  OfflineSession.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 01/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__OfflineSession__
#define __G_Ear_core__OfflineSession__

#include "ISession.h"
#include "AtomicPtr.h"
#include "SerialExecutor.h"
#include SHAREDFROMTHIS_H
#include ATOMIC_H

namespace Gear
{
    class OfflineSongArray;
    class PredicatePlaylist;
    
    class OfflineSession : public ISession, public MEMORY_NS::enable_shared_from_this<OfflineSession>
    {
    public:
#pragma message ("TODO: there is no such thing as an offline session, this is the wrong abstraction. Instead, we have to filter out songs when offline mode is enabled")

        //static shared_ptr<OfflineSession> create(const shared_ptr<IApp> &app);
        
        virtual shared_ptr<IPlaylist> libraryPlaylist();
        virtual void refresh();
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs);
        virtual shared_ptr<IPlaybackData> playbackDataSync(const ISong &song) const ;
        virtual ValidPtr<const vector<PlaylistCategory>> categories() const;
        
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<ISong> &song) const;
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<IPlaylist> &playlist) const;
        virtual void setOffline(bool offline);
        virtual std::string sessionIdentifier() const;
        virtual vector<int> possibleRatings() const;
        virtual bool saveForOfflinePossible() const;

    private:
        OfflineSession(const shared_ptr<IApp> &app);

        SignalConnection _updateConnection;
        
        bool _offline;
        shared_ptr<PredicatePlaylist> _savedForOfflinePlaylist;
        shared_ptr<PredicatePlaylist> _libraryPlaylist;
        shared_ptr<OfflineSongArray> _savedForOfflineArray;
        AtomicPtr<vector<PlaylistCategory>> _categories;
        vector<std::string> _previousPlaylistNames;
        
        THREAD_NS::mutex _fullMutex;
        AtomicPtr<vector<PlaylistCategory>> _fullCategories;
        ATOMIC_NS::atomic_int _updateCounter;
        Base::SerialExecutor _executor;
    };
}

#endif /* defined(__G_Ear_core__OfflineSession__) */
