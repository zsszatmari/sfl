//
//  BasicPlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/7/13.
//
//

#include <iostream>
#include "BasicPlaylist.h"
#include "Predicate.h"
#include "SortedSongArray.h"
#include "IApp.h"
#include "Db.h"
#include "SessionManager.h"
#include "IoService.h"

namespace Gear
{
#define method BasicPlaylist::
    
    method BasicPlaylist(const weak_ptr<ISession> &session, const string &name, const string &playlistId, const shared_ptr<ISongArray> &songArray, const shared_ptr<IPlaylist> previous, const std::map<string,string> &traits) :
    	IPlaylist(session),
        _name(name),
        _playlistId(playlistId),
        _songArray(songArray),
        _traits(traits)
    {
        _previous = previous;
    }
    
    const string method playlistId() const
    {
        return _playlistId;
    }
    
    const string method name() const
    {
        return _name;
    }
    
    void method setName(const string &name)
    {
    }
    
    const shared_ptr<ISongArray> method songArray()
    {
        auto array = _songArray;
        Io::get().post([array]{
            auto size = array->sizeSync();
            array->songsLeft(size);
        });
        return _songArray;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>();
    }
    
    
    bool method saveForOfflinePossible() const
    {
        return false;
    }

    void method setFilterPredicate(const SongPredicate &f)
    {
        if (_traits.find("freeSearch") != _traits.end()) {
            if (!f.key().empty()) {
                // filter locally nevertheless if searching for artist/etc...
                IPlaylist::setFilterPredicate(f);
            }
            IApp::instance()->sessionManager()->search(f.value());
        } else {
            IPlaylist::setFilterPredicate(f);
        }
    }

    shared_ptr<IPlaylist> playlistFrom(const ClientDb::Predicate &predicate,const string &playlistId, const string &name, const shared_ptr<ISession> &session, const std::map<string,string> &traits)
    {
        return shared_ptr<BasicPlaylist>(new BasicPlaylist(
                            session,
                            name,
                            playlistId,
                            shared_ptr<ISongArray>(new SortedSongArray(IApp::instance()->db(), predicate, traits)), nullptr, traits));
    }

    shared_ptr<IPlaylist> playlistFromData(const Json::Value &json)
    {
        auto source = json["source"].asString();
        auto playlistId = json["playlistId"].asString();
        auto isDynamic = json["dynamic"].asInt();
        if (isDynamic) {
#ifdef DEBUG
//            std::cout << "initing radio " << json["name"].asString() << "\n";
#endif
        }
        return playlistFrom(ClientDb::Predicate(ClientDb::Predicate::Operator::And,
                                                 {ClientDb::Predicate("source",source),
                                                  ClientDb::Predicate("playlist",playlistId)}),
                            playlistId,
                            json["name"].asString(),
                            IApp::instance()->sessionManager()->session(source),
                            isDynamic ? std::map<string,string>{{std::make_pair("dynamic","1"),
                                                                 std::make_pair("playlistId",playlistId),
                                                                 std::make_pair("source",source)}} : std::map<string,string>());
    }

    void method refresh()
    {
        if (_traits.find("dynamic") != _traits.end()) {
            auto session = _session.lock();
            if (session) {
                session->clearDynamic(_playlistId);
            }
        }
    }
}
