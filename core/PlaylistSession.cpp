//
//  PlaylistSession.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 6/14/13.
//
//

#include <functional>
#include "PlaylistSession.h"
#include "IPlaylist.h"
#include "ISong.h"

namespace Gear {
    
    using std::function;
    
#define method PlaylistSession::
    
    const function<void(const shared_ptr<IPlaylist> &)> PlaylistSession::discard = [](const shared_ptr<IPlaylist> &){};
    
    method PlaylistSession(const shared_ptr<IApp> &app) :
        ISession(app)
    {
    }
    
    void method createUserPlaylist(const vector<shared_ptr<ISong>> &songs, const function<void(const shared_ptr<IPlaylist> &)> &result)
    {
        if (songs.size() == 0) {
            const string name = u("New playlist");
            createUserPlaylist(songs, name, result);
        } else {
            createUserPlaylist(songs, songs.at(0)->title(), result);
        }
    }
    
    void method createUserPlaylist(const string &name, const function<void(const shared_ptr<IPlaylist> &)> &result)
    {
        createUserPlaylist(vector<shared_ptr<ISong>>(), name, result);
    }

    void method modifyCategoryByTag(int tag, const function<void(PlaylistCategory &)> &modify)
    {
        _categories.set([&](vector<PlaylistCategory> &categories){
            
            for (auto itCategory = categories.begin() ; itCategory != categories.end() ; ++itCategory) {
                auto &category = *itCategory;
                
                if (category.tag() == tag) {
                    modify(category);
                }
            }
        });
    }

    /*void method modifyCategoryByTag(int tag, const function<PlaylistCategory(const PlaylistCategory &)> &modify)
    {
        modifyCategoryByTag(tag, [&](PlaylistCategory &cat){
            cat = modify(cat);
        });
    }*/
    
    PlaylistCategory method readCategoryByTag(int tag) const
    {
        auto categories = _categories;
        for (auto itCategory = categories->begin() ; itCategory != categories->end() ; ++itCategory) {
        	auto &category = *itCategory;

        	if (category.tag() == tag) {
                return category;
            }
        }
        static PlaylistCategory noCategory;
        return noCategory;
    }
}
