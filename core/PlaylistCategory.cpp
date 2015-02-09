//
//  PlaylistCategory.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 7/10/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "PlaylistCategory.h"
#include "Color.h"
#include "ISession.h"
#include "NamedImage.h"
#include "json.h"
#include "RemotePlaylist.h"

namespace Gear
{
#define method PlaylistCategory::
    
    using namespace Gui;
    
    method PlaylistCategory() :
        _tag(-1),
        _defaultSelection(false),
        _singularPlaylist(false),
        _level(0),
        _presentation(Presentation::Normal)
    {
    }
    
    method PlaylistCategory(const string &title, int tag, bool defaultSelection) :
        _title(title),
        _defaultSelection(defaultSelection),
        _tag(tag),
        _singularPlaylist(false),
        _level(0),
        _presentation(Presentation::Normal)
    {
        switch(tag) {
            case kRadioTag:
                _icon = shared_ptr<IPaintable>(new NamedImage("category-radio"));
                break;
            case kAllTag:
                _icon = shared_ptr<IPaintable>(new NamedImage("category-library"));
                break;
            case kAutoPlaylistsTag:
                _icon = shared_ptr<IPaintable>(new NamedImage("category-star"));
                break;
            case kFreeSearchTag:
                _icon = shared_ptr<IPaintable>(new NamedImage("category-allaccess"));
                break;
            case kArtistsTag:
                _icon = shared_ptr<IPaintable>(new NamedImage("category-artists"));
                break;
            case kAlbumArtistsTag:
                _icon = shared_ptr<IPaintable>(new NamedImage("category-albumartists"));
                break;
            case kPlaylistsTag:
                _icon = shared_ptr<IPaintable>(new NamedImage("category-playlists"));
                break;
        }
    }

    bool method operator==(const PlaylistCategory &rhs) const
    {
        return _tag == rhs._tag;
    }

    method operator bool() const
    {
        return _tag >= 0;
    }
    
    shared_ptr<Gui::IPaintable> method icon() const
    {
        return _icon;
    }
    
    void method setSingularPlaylist(bool value)
    {
        _singularPlaylist = value;
    }
    
    bool method singularPlaylist() const
    {
        return _singularPlaylist;
    }
    
    int method tag() const
    {
        return _tag;
    }
    
    string method title() const
    {
        return _title;
    }

    void method setTitle(const string &title)
    {
        _title = title;
    }
    
    bool method defaultSelection() const
    {
        return _defaultSelection;
    }
    
    vector<shared_ptr<IPlaylist>> & method playlists()
    {
        return _playlists;
    }
    
    const vector<shared_ptr<IPlaylist>> & method playlists() const
    {
        return _playlists;
    }
    
    void method setLevel(int level)
    {
        _level = level;
    }
    
    int method level() const
    {
        return _level;
    }
    
    PlaylistCategory::Presentation method presentation() const
    {
        return _presentation;
    }
    
    void method setPresentation(PlaylistCategory::Presentation p)
    {
        _presentation = p;
    }

    method PlaylistCategory(const Json::Value &value) :
        _title(value.get("title","").asString()),
        _defaultSelection(value.get("defaultSelection",false).asBool()),
        _tag(value.get("tag",0).asInt()),
        _singularPlaylist(value.get("singularPlaylist","").asInt()),
        _level(value.get("level",0).asInt()),
        _presentation(static_cast<Presentation>(value.get("presentation",0).asInt())),
        _playlists(RemotePlaylist::deserialize(value.get("playlists",Json::arrayValue)))
    {
        auto icon = value.get("icon","").asString();
        if (!icon.empty()) {
            _icon = shared_ptr<NamedImage>(new NamedImage(icon));
        }
    }

    method operator Json::Value() const
    {
        Json::Value ret;
        ret["title"] = _title;
        ret["defaultSelection"] = _defaultSelection;
        ret["tag"] = _tag;
        ret["singularPlaylist"] = _singularPlaylist;
        ret["level"] = _level;
        ret["presentation"] = static_cast<int>(_presentation);
        ret["playlists"] = RemotePlaylist::serialize(_playlists);
        auto icon = MEMORY_NS::dynamic_pointer_cast<NamedImage>(_icon);
        if (icon) {
            ret["icon"] = icon->imageName();
        }
        
        #pragma message("icon is missing from playlist category serialization")
        return ret;
    }

    Json::Value method serialize(const vector<PlaylistCategory> &categories)
    {
        Json::Value ret(Json::arrayValue);
        for (auto &category : categories) {
            ret.append(category);
        }
        return ret;
    }

    vector<PlaylistCategory> method deserialize(const Json::Value &array)
    {
        std::vector<PlaylistCategory> ret;
        if (array.isArray()) {
            for (auto i = 0 ; i < array.size() ; i++) {
                ret.push_back(PlaylistCategory(array[i]));
            }
        }
        return ret;
    }

}
