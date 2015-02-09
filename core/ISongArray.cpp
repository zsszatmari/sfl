//
//  ISongArray.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/25/13.
//
//

#include "stdplus.h"
#include "ISongArray.h"
#include "SongPredicate.h"
#include "QueueSongArray.h"
#include "QueueIntent.h"
#include "sfl/Prelude.h"
#include "sfl/Vector.h"
#include "IApp.h"
#include "SessionManager.h"
#include "GooglePlaySession.h"
#include "DbSession.h"

#include "CreateRadioIntent.h"
#include "CreatePlaylistIntent.h"
#include "AddToPlaylistIntent.h"
#include "OfflineIntent.h"
#include "AllAccessToLibraryIntent.h"
#include "RemoveFromPlaylistIntent.h"
#include "RemoveFromLibraryIntent.h"

namespace Gear
{
    using std::make_pair;
    using namespace Base;
    using namespace sfl;
    
#define method ISongArray::
    
    const size_t ISongArray::NotFound = -1;

    method ISongArray()
    {

    }
    
    method ~ISongArray()
    {
    }
    
    bool method operator==(const ISongArray &rhs)
    {
        return (this == &rhs);
    }
    
    vector<string> method forbiddenColumns() const
    {
        return vector<string>();
    }


    static shared_ptr<ISongIntent> offlineIntent(const vector<SongEntry> &songs)
    {
        return OfflineIntent::create(songs);
    }

    static vector<shared_ptr<ISongIntent>> sessionSpecificSongIntents(const vector<SongEntry> &songs, const string &fieldHint, const shared_ptr<ISession> &session)
    {
        // this should come beofre the non-session specific (contrary to how it was before)
        auto googleSession = MEMORY_NS::dynamic_pointer_cast<GooglePlaySession>(session);
        auto app = IApp::instance();
        
        vector<shared_ptr<ISongIntent>> ret;
        
        if (app && googleSession && songs.size() == 1) {
            shared_ptr<ISongIntent> createRadio;
            auto song = songs.at(0);
            createRadio = shared_ptr<ISongIntent>(new CreateRadioIntent(googleSession, app, fieldHint));
            ret.push_back(createRadio);
        }
        
        auto off = offlineIntent(songs);
        if (off) {
            ret.push_back(off);
        }
        
        return ret;
    }

    static vector<pair<string, vector<shared_ptr<ISongIntent>>>> sessionSpecificSongIntentSubMenus(const vector<SongEntry> &songs, const shared_ptr<ISession> &session)
    {
        auto lSession = MEMORY_NS::dynamic_pointer_cast<PlaylistSession>(session);
        if (lSession && lSession->manipulationEnabled()) {
            auto playlists = lSession->playlists();
            vector<shared_ptr<ISongIntent>> playlistIntents;
            playlistIntents.reserve(playlists.size() + 1);
            for (auto it = playlists.begin() ; it != playlists.end() ; ++it) {
                auto &playlist = *it;
                playlistIntents.push_back(shared_ptr<ISongIntent>(new AddToPlaylistIntent(playlist)));
            }
            playlistIntents.push_back(shared_ptr<ISongIntent>(new CreatePlaylistIntent(lSession)));
            
            pair<string, vector<shared_ptr<ISongIntent>>> p = make_pair(u("Add to Playlist"), playlistIntents);
            return Vector::singleton(p);
        } else {
            return Vector::empty<pair<string, vector<shared_ptr<ISongIntent>>>>();
        }
    }
    

    static vector<shared_ptr<ISongIntent>> addRemoveSongIntents(const vector<SongEntry> &songs, const shared_ptr<ISession> &session)
    {
        bool thisIsLibrary = songs.size() > 0 && songs.at(0).song()->stringForKey("playlistId") == "all";       

        // if one has a library, it should be editable. it's his/her personal library after all 
        bool shouldAddToLibrary = !thisIsLibrary;
        bool shouldRemoveFromLibrary = thisIsLibrary;

#pragma message("TODO: don't allow remove items from radios, all access search, and other exotic stuff")
        bool shouldRemoveFromPlaylist = !thisIsLibrary;
        
        auto empty = Vector::empty<shared_ptr<ISongIntent>>();
        auto results = 
            (shouldAddToLibrary ? Vector::singleton(shared_ptr<ISongIntent>(new AllAccessToLibraryIntent(session))) : empty) +
            (shouldRemoveFromLibrary ? Vector::singleton(shared_ptr<ISongIntent>(new RemoveFromPlaylistIntent(session))) : empty) +
            (shouldRemoveFromPlaylist ? Vector::singleton(shared_ptr<ISongIntent>(new RemoveFromLibraryIntent(session))) : empty);
        return results;
    }

    static vector<shared_ptr<ISongIntent>> queueIntents(bool isThisQueue)
    {
        vector<shared_ptr<ISongIntent>> ret;
        if (!isThisQueue) {
            ret.push_back(shared_ptr<ISongIntent>(new QueueIntent(QueueIntent::Flavour::EnqueueLast)));
            ret.push_back(shared_ptr<ISongIntent>(new QueueIntent(QueueIntent::Flavour::EnqueueNext)));
        } else {
            ret.push_back(shared_ptr<ISongIntent>(new QueueIntent(QueueIntent::Flavour::Remove)));
        }
        return ret;
    }

    static vector<shared_ptr<ISession>> sessionsForSongs(const vector<SongEntry> &songs)
    {
        auto sources = ordNub(sfl::map([](const SongEntry &entry){return entry.song()->stringForKey("source");}, songs));
        return sfl::map([](const std::string &source){return IApp::instance()->sessionManager()->session(source);}, sources);
    }

    vector<shared_ptr<ISongIntent>> method songIntents(const vector<SongEntry> &songs, const string &fieldHint) const
    {
        auto sessions = sessionsForSongs(songs);

#pragma message("TODO: search in all access and search for artist...")

#pragma message("TODO: make enqueues work with remote")        

        return concat(sfl::map(std::bind(sessionSpecificSongIntents,songs,fieldHint,std::placeholders::_1),sessions)) +
               concat(sfl::map(std::bind(addRemoveSongIntents,songs,std::placeholders::_1),sessions)) +
               queueIntents(this == QueueSongArray::instance().get());
    }

    vector<pair<string, vector<shared_ptr<ISongIntent>>>> method songIntentSubMenus(const vector<SongEntry> &songs) const
    {
        auto sessions = sessionsForSongs(songs);

        if (sessions.size() == 1) {
            return sessionSpecificSongIntentSubMenus(songs,sessions[0]);
        } else {
            return {};
        }
    }
    
    const string method emptyText() const
    {
        return u("");
    }
    
    void method songsLeft(unsigned long count)
    {
    }
    
    const bool method orderedArray() const
    {
        return false;
    }
    
    void method freeUpMemory()
    {
    }
    
    bool method needsPredicate() const
    {
        return false;
    }
    
    void method setSecondaryFilterPredicate(const SongPredicate &f)
    {
    }
    
    SongPredicate method secondaryFilterPredicate() const
    {
        return SongPredicate();
    }

    SortedSongArray & method impl()
    {
        return *static_cast<SortedSongArray *>(this);
    }
    
    // hopefully we won't need this, seems slow
    /*
    const vector<SongEntry> method visibleSongs() const
    {
        auto s = size();
        vector<SongEntry> entries;
        entries.reserve(s);
        for (int i = 0 ; i < s ; ++i) {
            entries.push_back(at(i));
        }
        return entries;
    }*/
    
}
