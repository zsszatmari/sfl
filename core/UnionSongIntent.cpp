//
//  UnionSongIntent.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/5/13.
//
//

#include "UnionSongIntent.h"
#include "sfl/Maybe.h"
#include "sfl/Vector.h"
#include "SongEntry.h"

namespace Gear
{
#define method UnionSongIntent::
    
    using namespace sfl;

    method UnionSongIntent(const vector<shared_ptr<ISongIntent>> &intents) :
        _intents(intents)
    {
    }
    
    const string method menuText() const
    {
        if (_intents.size() > 0) {
            return _intents.at(0)->menuText();
        }
        return "";
    }
    
    void method apply(const vector<SongEntry> &songs) const
    {
#ifdef DEBUG
        //std::cout << "apply " << _intents.size() << " intents to " << songs.size() << std::endl;
#endif

        // it would be erroneus to apply e.g. download for offline n times on _all_ n songs, which is n squared
        // so, just apply on the first

        auto maybeIntent = maybeAt(_intents,0);
        //std::function<std::function<void()>(shared_ptr<ISongIntent>)> 
		auto mapFunction = [=](const shared_ptr<ISongIntent> &intent){  // !!!
            return std::function<void()>([=]{
                intent->apply(songs);
            });
        };
        //auto maybeAction = fmap<decltype(mapFunction),decltype(maybeIntent),shared_ptr<ISongIntent>,std::function<void()>>(std::move(mapFunction), std::move(maybeIntent));
        //auto maybeAction = fmap(mapFunction, maybeIntent);
		auto maybeAction = match<Maybe<std::function<void()>>>(maybeIntent, [](const Nothing &)->Maybe<std::function<void()>>{return Nothing(); },
												[&](const shared_ptr<ISongIntent> &a)->Maybe< std::function<void()>>{return Nothing(); });
        auto action = maybe([]{}, maybeAction); 
        action();
    }
    
    bool method confirmationNeeded(const vector<SongEntry> &songs) const
    {
        for (auto &intent : _intents) {
            if (intent->confirmationNeeded(songs)) {
                return true;
            }
        }
        return false;
    }
    
    string method confirmationText(const vector<SongEntry> &songs) const
    {
        for (auto &intent : _intents) {
            if (intent->confirmationNeeded(songs)) {
                return intent->confirmationText(songs);
            }
        }
        return "";
    }
}
