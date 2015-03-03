//
//  ISession.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#ifndef __G_Ear__ISession__
#define __G_Ear__ISession__

#include <vector>
#include "GearUtility.h"
#include "EventSignal.h"
#include "ManagedValue.h"
#include "ValidPtr.h"

namespace Gui
{
	class IPaintable;
}

namespace Gear
{
    extern const char kAllCategory[];
    extern const char kPlaylistsCategory[];
    extern const char kAutoPlaylistsCategory[];
    extern const char kSubscriptionsCategory[];
    
    // this must be in the order of appereance
    const int kFreeSearchTag = 1;
    const int kAllTag = 2;
    const int kArtistsTag = 3;
    const int kAlbumArtistsTag = 4;
    const int kPlaylistsTag = 5;
    const int kRadioTag = 6;
    const int kSubscriptionsTag = 7;
    const int kAutoPlaylistsTag = 8;
    
    using namespace Base;
    using std::vector;
    using std::function;
    
    class PlaylistCategory;
    class IApp;
    class IPlaylist;
    class IPlaybackData;
    class ISong;
    class SongEntry;
    
    class core_export ISession
    {        
    public:
        ISession(const shared_ptr<IApp> &app);
        // can call this with the same token until returns empty
        //virtual const vector<SongEntry> searchSync(const std::string &filter, std::string &token);

        PlaylistCategory categoryByTag(int tag);
        
        virtual ~ISession();
        // needed by RemoveFromLibraryIntent
        virtual shared_ptr<IPlaylist> libraryPlaylist() = 0;
        virtual void refresh() = 0;
        virtual void addAllAccessToLibrary(const vector<shared_ptr<ISong>> &songs) = 0;
        virtual void playbackData(const ISong &song, const function<void(const shared_ptr<IPlaybackData> &)> data) const = 0;
        virtual ValidPtr<const vector<PlaylistCategory>> categories() const = 0;
        EventConnector connectedEvent();
        EventConnector playlistsChangeEvent();
        ValueConnector<bool> refreshingConnector();
        virtual bool manipulationEnabled() const;
        // there is a default (0,1,5) implementation, but still explicitly implement it to be sure
        virtual vector<int> possibleRatings() const = 0;
 
        const shared_ptr<IApp> app();
        virtual std::string sessionIdentifier() const;
        
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<ISong> &song) const = 0;
        virtual shared_ptr<Gui::IPaintable> sessionIcon(const shared_ptr<IPlaylist> &playlist) const = 0;

        virtual void freeUpMemory();
        virtual void setOffline(bool offline);
        virtual bool saveForOfflinePossible() const;
        virtual void dispose();
        
    protected:
        EventSignal _connectedSignal;
        EventSignal _playlistsChangeSignal;
        ManagedValue<bool> _refreshing;
        
    private:
        ISession(const ISession &rhs); // delete
        const ISession &operator=(const ISession &rhs); // delete
    
        const shared_ptr<IApp> _app;
    };
}

#endif /* defined(__G_Ear__ISession__) */
