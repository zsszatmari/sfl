//
//  SessionManager.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/6/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__SessionManager__
#define __G_Ear_iOS__SessionManager__

#include "stdplus.h"
#include "ISession.h"
#include "AtomicPtr.h"
#include "PlaylistCategory.h"
#include UNORDERED_MAP_H
#include ATOMIC_H
#include SHAREDFROMTHIS_H
#include "SerialExecutor.h"

namespace Gear
{
    class UnionPlaylist;
    class ICategoriesTransformer;
    
    // this is considered a session by itself because everything that can be done with a session can be done with it
    class core_export SessionManager final : public ISession, public MEMORY_NS::enable_shared_from_this<SessionManager>
    {
    public:
        static shared_ptr<SessionManager> create(const shared_ptr<IApp> &app);
        virtual const vector<SongEntry> searchSync(const std::string &filter, std::string &token);
        shared_ptr<IPlaybackData> playbackDataSync(const ISong &song) const;
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs);
        virtual shared_ptr<IPlaylist> libraryPlaylist();
        const shared_ptr<IPlaylist> freeSearchPlaylist();
        virtual ValidPtr<const vector<PlaylistCategory>> categories() const;
        virtual void refresh();
        
        void addSession(const shared_ptr<ISession> &session);
        void removeSession(const shared_ptr<ISession> &session);
        
        shared_ptr<ISession> session(const std::string &identifier) const;
        
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<ISong> &song) const;
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<IPlaylist> &playlist) const;
        
        virtual void freeUpMemory();
        virtual void setOffline(bool offline);
        virtual std::string sessionIdentifier() const;
        void recomputeCategories();
        virtual vector<int> possibleRatings() const;
        
    private:
        SessionManager(const shared_ptr<IApp> &app);
        
        void reselectCurrentPlaylist();
        void onConnected();
        void onPlaylistChange();
        void onRefreshingChange(bool value);
        
        AtomicPtr<vector<shared_ptr<ISession>>> _sessions;
        const shared_ptr<ICategoriesTransformer> _categoriesTransformer;
        ATOMIC_NS::atomic<bool> _offline;
        SignalConnection _fiveStarConnection;
        
        struct Unions
        {
            vector<PlaylistCategory> categories;
            UNORDERED_NS::unordered_map<std::string, shared_ptr<UnionPlaylist>> playlists;
            shared_ptr<UnionPlaylist> libraryPlaylist;
            shared_ptr<UnionPlaylist> thumbsUpPlaylist; 
            shared_ptr<IPlaylist> highlyRatedPlaylist;
            
            shared_ptr<IPlaylist> artistsForPhone;
        };
        static Unions createUnions();
        
        const Unions _unions;
        shared_ptr<IPlaylist> _albumArtistsForPhone;
        
        const shared_ptr<ISession> _offlineSession;
        ValidPtr<const vector<PlaylistCategory>> doRecomputeCategories();
        
        //vector<string> _removeFromAutoIfEmpty;
        
        mutable ATOMIC_NS::atomic<bool> _needSessionIcons;

        Base::SerialExecutor _recomputeExecutor;
        shared_ptr<bool> _recomputeState;
        AtomicPtr<vector<PlaylistCategory>> _categories;
    };
}

#endif /* defined(__G_Ear_iOS__SessionManager__) */
