//
//  PhoneCategoriesTransformer.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/12/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include <iostream>
#include <sstream>
#include "PhoneCategoriesTransformer.h"
#include "GooglePlayRadio.h"
#include "ISession.h"
#include "GroupingAsPlaylist.h"
#include "GroupedPlaylist.h"

namespace Gear
{
    using namespace Base;
    
#define method PhoneCategoriesTransformer::
    
    static PlaylistCategory artistsCategory(const shared_ptr<IPlaylist> &playlist)
    {
        // either it is a GroupedPlaylist, or a RemotePlaylist
        PlaylistCategory f;
        //std::cout << "phonecategories converting " << playlist->playlistId() << std::endl;

        if (playlist->playlistId() == "GROUPED:albumArtist") {
            f = PlaylistCategory("A. Artists", kAlbumArtistsTag);
        } else {
            f = PlaylistCategory("Artists", kArtistsTag);
        }
        f.setSingularPlaylist(false);
        
        const vector<SongGrouping> &groupings = playlist->availableGroupingsConnector().value();
        for (auto it = groupings.begin() ; it != groupings.end() ; ++it) {
            
            #pragma message("TODO: fix grouping by artists on iphone (including remote control)")
            f.playlists().push_back(GroupingAsPlaylist::create(playlist, *it));
        }
        return f;
    }
    
    vector<PlaylistCategory> method transform(const vector<PlaylistCategory> &original) const
    {
        vector<PlaylistCategory> transformed;
        
        for (auto it = original.begin() ; it != original.end() ; ++it) {
            if (it->tag() == kAllTag) {
                
                auto &playlists = it->playlists();
                for (auto itPlaylist = playlists.begin() ; itPlaylist != playlists.end() ; ++itPlaylist) {
                    
                    auto &playlist = *itPlaylist;
                    
                    if (playlist->playlistId() == "all") {
                        PlaylistCategory all(playlist->name(), kAllTag, true);
                        all.setSingularPlaylist(true);
                        all.playlists().push_back(playlist);
                        transformed.push_back(all);
                    } else if (playlist->playlistId() == "free") {
                        PlaylistCategory f(playlist->name(), kFreeSearchTag);
                        f.playlists().push_back(playlist);
                        f.setSingularPlaylist(true);
                        transformed.insert(transformed.begin(), f);
                    } else if (playlist->playlistId() == "GROUPED:artist" ||
                               playlist->playlistId() == "GROUPED:albumArtist") {
                    
                        // the static_cast is wrong here, because it can be also be a RemotePlaylist!
                        transformed.push_back(artistsCategory(playlist));
                        //transformed.push_back(artistsCategory(MEMORY_NS::static_pointer_cast<GroupedPlaylist>(playlist)));
                    }
                }
            } else if (it->tag() == kAutoPlaylistsTag) {
                auto cat = *it;
                cat.setTitle("Auto");
                transformed.push_back(cat);
            } else {
                transformed.push_back(*it);
            }
        }
        
        
        return transformed;
    }
}
