//
//  PlaylistCategory.h
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/10/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__PlaylistCategory__
#define __G_Ear_core__PlaylistCategory__

#include <string>
#include <vector>
#include "json-forwards.h"
#include "Environment.h"
#include "stdplus.h"
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gui
{
    class IPaintable;
}

namespace Gear
{
    using std::string;
    using std::vector;
    
    class IPlaylist;
    
    class core_export PlaylistCategory final
    {
    public:
        PlaylistCategory();
        PlaylistCategory(const string &title, int tag, bool defaultSelection = false);
        
        string title() const;
        explicit operator bool() const;
        void setTitle(const string &title);
        bool defaultSelection() const;
        
        // supports modifying, thus not const
        vector<shared_ptr<IPlaylist>> &playlists();
        const vector<shared_ptr<IPlaylist>> &playlists() const;
        int tag() const;
        bool singularPlaylist() const;
        void setSingularPlaylist(bool value);
        shared_ptr<Gui::IPaintable> icon() const;
        void setLevel(int level);
        int level() const;
        
        enum class Presentation
        {
            Normal,
            Albums
        };
        Presentation presentation() const;
        void setPresentation(Presentation p);
    	bool operator==(const PlaylistCategory &rhs) const;
        PlaylistCategory(const Json::Value &json);
        operator Json::Value() const;
        static Json::Value serialize(const vector<PlaylistCategory> &);
        static vector<PlaylistCategory> deserialize(const Json::Value &array);
        
    private:
        string _title;
        bool _defaultSelection;
        vector<shared_ptr<IPlaylist>> _playlists;
        int _tag;
        bool _singularPlaylist;
        shared_ptr<Gui::IPaintable> _icon;
        int _level;
        Presentation _presentation;
    };
}

#endif /* defined(__G_Ear_core__PlaylistCategory__) */
