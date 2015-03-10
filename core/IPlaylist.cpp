//
//  IPlaylist.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/4/13.
//
//

#include "IPlaylist.h"
#include "ISongArray.h"
#include "ImageToDeliver.h"
#include "Color.h"
#include "IApp.h"
#include "AlbumArtStash.h"
#include "NamedImage.h"
#include "PlaylistDeleteIntent.h"
#include "OfflinePlaylistIntent.h"
#include "OfflineIntent.h"
#include "ISession.h"
#include "Chain.h"
#include "ValidPtr.h"
#include "OfflineService.h"
#include "IPreferences.h"
#include "SongSortOrder.h"
#include "UnionPlaylistIntent.h"
#include "SortedSongArray.h"
#include "YouTubeSession.h"
#include "ThemeManager.h"
#include "ITheme.h"

namespace Gear
{
#define method IPlaylist::
    
    using namespace Gui;
    
    method IPlaylist(const weak_ptr<ISession> &session) :
        _ordered(true),
        _session(session)
    {
    }
    
    shared_ptr<ISession> method session() const
    {
    	return _session.lock();
    }

    method ~IPlaylist()
    {
    }
    
    bool method removable() const
    {
        return false;
    }
    
    void method remove()
    {
    }
    
    bool method editable() const
    {
        return false;
    }
    
    void method addSongEntries(const vector<SongEntry> &songs)
    {
    }
    
    void method removeSongs(const vector<SongEntry> &songs)
    {
        auto sortedArray = MEMORY_NS::dynamic_pointer_cast<SortedSongArray>(songArray());
        if (sortedArray) {
            sortedArray->removeSongs(songs);
        }
    }
    
    bool method moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before)
    {
        auto sortedArray = MEMORY_NS::dynamic_pointer_cast<SortedSongArray>(songArray());
        if (sortedArray) {
            return sortedArray->moveSongs(entries, after, before);
        }
        return false;
    }
    
    bool method orderedPlaylist() const
    {
        return _ordered;
    }
    
    void method setOrdered(bool ordered)
    {
        _ordered = ordered;
    }
    
    bool method operator==(const IPlaylist &rhs) const
    {
        return playlistId() == rhs.playlistId();
    }
    
    bool method groupingsEnabled() const
    {
        return false;
    }

    Base::ValueConnector<SongGrouping> method selectedGroupingConnector()
    {
        return _selectedGrouping.connector();
    }
    
    Base::ValueConnector<vector<SongGrouping>> method availableGroupingsConnector()
    {
        return _availableGroupings.connector();
    }
    
    void method selectGrouping(const SongGrouping &grouping, bool play)
    {
        _selectedGrouping = grouping;
    }
    
    void method setFilterPredicate(const SongPredicate &f)
    {
        _filterPredicate = f;
        songArray()->setFilterPredicate(f);
    }
    
    const SongPredicate & method filterPredicate() const
    {
        return _filterPredicate;
    }
    
    shared_ptr<PromisedImage> method emptyImage()
    {
        shared_ptr<ImageToDeliver> image = shared_ptr<ImageToDeliver>(new ImageToDeliver(0));
        
        image->setImage(IApp::instance()->themeManager()->current()->noArt(), ImageToDeliver::Rating::NoArt);
        //image->setImage(shared_ptr<Color>(new Color(0,0,0,0)));
        return image;
    }
    
    const shared_ptr<const ISongArray> method songArray() const
    {
        return const_cast<IPlaylist *>(static_cast<const IPlaylist *>(this))->songArray();
    }
    
    shared_ptr<PromisedImage> method image(int preferredSize) const
    {
        auto s = songArray();
        #pragma message("TODO: album art url for playlist")
        /*if (s) {
            auto songs = s->songs();
            if (songs->size() > 0) {
                return IApp::instance()->albumArtStash()->art(songs->at(0).song(), preferredSize);
            }
        }*/
        return emptyImage();
    }
    
    bool method saveForOfflineAsPlaylist()
    {
        return IApp::instance()->selectedPlaylistConnector().value().first.tag() == kPlaylistsTag;
    }
    
    vector<shared_ptr<IPlaylistIntent>> method playlistIntents()
    {
        vector<shared_ptr<IPlaylistIntent>> ret;
#if TARGET_OS_IPHONE || !defined(DISABLE_OFFLINE_ON_DESKTOP)
        if (saveForOfflinePossible() && OfflineService::available() && session() && session()->saveForOfflinePossible()) {
            
            bool asPlaylist = saveForOfflineAsPlaylist();
            std::string n = asPlaylist ? name() : "";
            
            ret.push_back(shared_ptr<IPlaylistIntent>(new OfflinePlaylistIntent(intentApplyArray(), true, n)));
            ret.push_back(shared_ptr<IPlaylistIntent>(new OfflinePlaylistIntent(intentApplyArray(), false, n)));
        }
#endif
        return ret;
    }
    
    shared_ptr<const ISongArray> method intentApplyArray() const
    {
        return songArray();
    }
    
    std::pair<PlaylistCategory,shared_ptr<IPlaylist>> method selected()
    {
        return std::make_pair<PlaylistCategory,shared_ptr<IPlaylist>>(PlaylistCategory(), nullptr);
    }

    static string sortPreferenceKey(const string &id)
    {
        const string kPrefixGrouping = "GROUPED:";
        const string kPrefixEquals = "CONDITION/";
        const string uniques[] = {"all", "free", "LASTDAYS","all","free","savedforoffline", "queue", "LASTADDED"};
        const int N = sizeof(uniques)/sizeof(string);
        
        if (id.compare(0, kPrefixGrouping.length(), kPrefixGrouping) == 0
            || id.compare(0, kPrefixEquals.length(), kPrefixEquals) == 0
            || find(uniques, uniques + N, id) != (uniques + N)) {

            return string("SortDescriptorUnique") + id;
        } else {
            return "SortDescriptorGlobal";
        }
    }
    
    void method setSortDescriptor(const SortDescriptor &rhs)
    {
        songArray()->setSortDescriptor(rhs);

        if (keepSongOrder()) {
            string key = sortPreferenceKey(playlistId());
            string value = rhs.serialize();

            //std::cout << "sort saving " << key << " (" << playlistId() << ") => " << value << std::endl;
            IApp::instance()->preferences().setStringForKey(key, value);
        }
    }

    SortDescriptor method sortDescriptorForId(const string &playlistId)
    {
        auto key = sortPreferenceKey(playlistId);
        auto value = SortDescriptor(IApp::instance()->preferences().stringForKey(key));
        //std::cout << "sort loading " << key << " => " << value.serialize() << std::endl;
        return value;
    }

    SortDescriptor method sortDescriptorGlobal()
    {
        return sortDescriptorForId("");
    }

    std::pair<string,bool> method sortKeyAndAscending() const
    {
        auto keys = songArray()->sortDescriptor().keysAndAscending();
        if (keys.empty()) {
            return std::make_pair(SongSortOrder::defaultSortOrder().front(), true);
        } else {
            return keys.front();
        }
    }

    bool method keepSongOrder() const
    {
        return true;
    }

    vector<shared_ptr<IPlaylistIntent>> method playlistIntents(const vector<shared_ptr<IPlaylist>> &playlists)
    {
        vector<shared_ptr<IPlaylistIntent>> ret;
        if (playlists.empty()) {
            return ret;
        }

        std::map<string,vector<shared_ptr<IPlaylistIntent>>> collected;
        for (auto &playlist : playlists) {
            auto intents = playlist->playlistIntents();
            for (auto &retIntent : intents) {
                auto str = retIntent->menuText();

                collected[str].push_back(retIntent);
            }
        }

        for (auto &p : collected) {
            if (p.second.size() == playlists.size()) {
                ret.push_back(shared_ptr<IPlaylistIntent>(new UnionPlaylistIntent(p.first, p.second)));
            }  
        }
        return ret;
    }

    bool method nameEditable() const
    {
        return editable();
    }

    bool method reorderable() const
    {
        if (MEMORY_NS::dynamic_pointer_cast<YouTubeSession>(session())) {
            return false;
        }
        return true;
    }

    void method refresh()
    {
    }

    shared_ptr<IPlaylist> method previous() const
    {
        return _previous;
    }


    /*
    bool method saveForOfflineAsPlaylist() const
    {
        return false;
    }*/
}
