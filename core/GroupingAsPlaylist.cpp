//
//  GroupingAsPlaylist.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 9/25/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "GroupingAsPlaylist.h"
#include "GroupedPlaylist.h"
#include "IApp.h"
#include "ISongArray.h"
#include "SongEntry.h"
#include "ValidPtr.h"
#include "Chain.h"
#include "ModifiablePlaylist.h"
#include "SongSortOrder.h"
#include "IDb.h"

//#define DEBUG_COUNTER

namespace Gear
{
#define method GroupingAsPlaylist::
    
    shared_ptr<GroupingAsPlaylist> method create(const shared_ptr<IPlaylist> &playlist, const SongGrouping &grouping)
    {
        return shared_ptr<GroupingAsPlaylist>(new GroupingAsPlaylist(playlist, grouping));
    }
    
#ifdef DEBUG_COUNTER
    static int count = 0;
#endif
    
    method GroupingAsPlaylist(const shared_ptr<IPlaylist> &playlist, const SongGrouping &grouping) :
		IPlaylist(playlist->session()),
        _playlist(playlist),
        _grouping(grouping)
    {
#ifdef DEBUG_COUNTER
        count++;
        if (count % 1 == 0)
            std::cout << "groupingasplaylist inc: " << count << " (" << (int)(this) << ")" << std::endl;
#endif
    }
    
    method ~GroupingAsPlaylist()
    {
#ifdef DEBUG_COUNTER
        count--;
        std::cout << "groupingasplaylist dec: " << count << " (" << (int)(this) << ")" << std::endl;
#endif
    }
    
    /*
    shared_ptr<ISession> method session() const
    {
    	return _grouping.session();
    }*/

    const string method playlistId() const
    {
        return _playlist->playlistId();
    }
    
    const string method name() const
    {
        return _grouping.title();
    }
    
    void method setName(const string &name)
    {
    }
    
    const shared_ptr<ISongArray> method songArray()
    {
        if (!_songArray) {
            using ClientDb::Predicate;
            auto sorted = MEMORY_NS::dynamic_pointer_cast<SortedSongArray>(_playlist->songArray());
            ClientDb::Predicate pred = Predicate::compositeAnd(sorted->inherentPredicate(), _grouping.predicate());
            _songArray = shared_ptr<SortedSongArray>(new SortedSongArray(sorted->db(), pred));
        }
        return _songArray;
        //return _playlist->songArray();
    }
    
    std::pair<PlaylistCategory,shared_ptr<IPlaylist>> method selected()
    {
        // selection is triggered here!
        if (_grouping != _playlist->selectedGroupingConnector().value()) {
            
            if (IApp::instance()->phoneInterface() && (_grouping.firstField() == "artist" || _grouping.firstField() == "albumArtist")) {
            //if (false) {
                // there should be a subgrouping present here: artist > album
                
                PlaylistCategory selectedCategory = IApp::instance()->selectedPlaylistConnector().value().first;
                PlaylistCategory albumListCategory(selectedCategory.title(), selectedCategory.tag(), false);
                albumListCategory.setLevel(1);
                albumListCategory.setPresentation(PlaylistCategory::Presentation::Albums);
                
                SongPredicate predicate = _grouping.predicate();

                auto sorted = MEMORY_NS::dynamic_pointer_cast<SortedSongArray>(_playlist->songArray());
                auto sortDescriptor = sorted->sortDescriptor();
                using ClientDb::Predicate;
                ClientDb::Predicate pred = Predicate::compositeAnd(sorted->inherentPredicate(), _grouping.predicate());
            
                auto data = sorted->db()->fetchGroupings("Song", predicate, sortDescriptor, {"album"}, {"album"});
                for (auto &item : data) {
#pragma message("TODO: make artist->album view work")
                    /*auto playlist = BasicPlaylist::create("", nullptr);
                    //set song array
                    playlist->setOrdered(true);
                    playlist->addSongsLocally(albumSongs);
                    playlist->setName(it->title());
                    playlist->setRemovable(false);
                    playlist->setEditable(false);
                    playlist->setSaveForOfflinePossible(true);
                    playlist->storedSongArray()->setSongIntentProxy(_playlist->unfilteredArray());

                    albumListCategory.playlists().push_back(playlist);*/
                }
                
                return std::make_pair(albumListCategory, nullptr);
            } else {
                _playlist->selectGrouping(_grouping);
            }
        }
        
        return std::make_pair(PlaylistCategory(), nullptr);
    }
    
    shared_ptr<const ISongArray> method intentApplyArray() const
    {
        if (!_intentApplyArray) {
            #pragma message("TODO: GroupingAsPlaylist intentApplyArray() not working")
            /*vector<SongEntry> songs = *(_playlist->unfilteredArray()->rawSongs());
            vector<SongEntry> filteredSongs;
            
            SongPredicate predicate = _grouping.predicate();
            remove_copy_if(songs.begin(), songs.end(), back_inserter(filteredSongs), [&](const SongEntry &entry){
                return !predicate.test(*entry.song());
            });

            
            // create temporary playlist
            auto playlist = ModifiablePlaylist::create("", nullptr);
            playlist->setOrdered(true);
            playlist->addSongsLocally(filteredSongs);
            
            // must retain otherwise it dies instantly
            _intentApplyArray = playlist->songArray();*/
        }
        return _intentApplyArray;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>();
    }
    
    shared_ptr<PromisedImage> method image(int preferredSize) const
    {
        auto images = _grouping.images(preferredSize);
        if (images.empty()) {
            return shared_ptr<PromisedImage>();
        }
        return images.at(0);
    }

    bool method saveForOfflinePossible() const
    {
        return true;
    }
}
