//
//  GroupedPlaylist.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/1/13.
//
//

#include <iostream>
#include "GroupedPlaylist.h"
#include "SortedSongArray.h"
#include "MusicSort.h"
#include "BackgroundExecutor.h"
#include "IApp.h"
#include "CoreDebug.h"
#include "SessionManager.h"
#include "BaseUtility.h"
#include "IoService.h"
#include "IPreferences.h"
#include "Db.h"
#include "BasicPlaylist.h"

namespace Gear
{
    
#define method GroupedPlaylist::
    
    using std::string;
    using namespace Base;
    
    
    const string method playlistId() const
    {
        return _playlistId;
    }
    
    const string method name() const
    {
        return _name;
    }
    
    static string createId(const vector<string> &groupBy)
    {
        string ret = "GROUPED";
        for (auto it = groupBy.begin() ; it != groupBy.end() ; ++it) {
        	auto &g = *it;
        	ret.append(":");
            ret.append(g);
        }
        return ret;
    }
    
    
    // filter out everything to avoid sort-related overhaad
    static SongPredicate undefined()
    {
        return SongPredicate("artist","undefined",SongPredicate::Matches());
    }
    
    method GroupedPlaylist(const shared_ptr<SortedSongArray> &base, const string &name, const vector<string> &groupBy, const shared_ptr<IPlaylist> &previous) :
        IPlaylist(weak_ptr<ISession>()),
        _name(name),
        _playlistId(createId(groupBy)),
        _array(base),
        _groupBy(groupBy),
        _calculateId(0)
    {
        _previous = previous;
    }
    
    void method setName(const string &name)
    {
    }
    
    const shared_ptr<ISongArray> method songArray()
    {
        return nullptr;
    }
    
    shared_ptr<ISongNoIndexIntent> method dragIntentTo(const vector<SongEntry> &songs)
    {
        return shared_ptr<ISongNoIndexIntent>();
    }
    
    shared_ptr<GroupedPlaylist> method create(const shared_ptr<SortedSongArray> &base, const string &name, const vector<string> &groupBy, const shared_ptr<IPlaylist> &previous)
    {
        shared_ptr<GroupedPlaylist> ret(new GroupedPlaylist(base, name, groupBy, previous));
        
        base->updatedEvent().connect(&GroupedPlaylist::recalculate, ret);
        ret->recalculate();
        weak_ptr<GroupedPlaylist> wself = ret;
        ret->_resortConnection = IApp::instance()->preferenceConnect("SortAlbumsByAlbumName",[wself]{
            auto self = wself.lock();
            if (self) {
                self->recalculate();
            }
        });
        
        return ret;
    }
    
    /*
    shared_ptr<GroupedPlaylist> method albumsPlaylist(const shared_ptr<SortedSongArray> &base)
    {
        string s[] = {"album", "artist"};
        return create(base, "Albums", Base::init<vector<string>>(s));
    }
    
    shared_ptr<GroupedPlaylist> method artistsPlaylist(const shared_ptr<SortedSongArray> &base)
    {
        string s[] = {"artist"};
        return create(base, "Artists", Base::init<vector<string>>(s));
    }
    
    shared_ptr<GroupedPlaylist> method genresPlaylist(const shared_ptr<SortedSongArray> &base)
    {
        string s[] = {"genre"};
        return create(base, "Genres", Base::init<vector<string>>(s));
    }
    
    shared_ptr<GroupedPlaylist> method albumArtistsPlaylist(const shared_ptr<SortedSongArray> &base)
    {
        string s[] = {"albumArtist"};
        return create(base, "Album Artists", Base::init<vector<string>>(s));
    }*/
    
    bool method groupingsEnabled() const
    {
        //std::cout << "groupings trigger\n";
        return true;
    }
    
    void method selectGrouping(const SongGrouping &grouping, bool play)
    {
#ifdef DEBUG
        std::cout << "select grouping: " << grouping.title() << std::endl;
#endif
        
        IPlaylist::selectGrouping(grouping, play);
        
        auto playlist = grouping.playlist();
        if (playlist) {
            IApp::instance()->userSelectedPlaylist(playlist, play);
        }
    }
    
    void method setFilterPredicate(const SongPredicate &predicate)
    {
        _predicate = predicate;
        recalculate();
    }

    vector<SongGrouping> method groupSongs(const ClientDb::Predicate &aPredicate, const vector<string> &groupBy)
    {
        using ClientDb::Predicate;

        auto db = IApp::instance()->db();

        std::vector<Json::Value> data;
        
        auto predicate = Predicate::compositeAnd(aPredicate, Predicate(Predicate::Operator::NonEmpty, groupBy.at(0),Json::Value()));
        bool multiImage = groupBy.size() == 1;

        // it's not clear why would we need session for SongGrouping
        //auto sess = session();
        if (groupBy.size() == 0 
            //|| !sess
            ) {
            return vector<SongGrouping>();
        }

        auto groupKey = groupBy.at(0);

        if (groupBy.size() >= 2) {
            // albums:  (album, artist)
            //SELECT album,artist,albumArtUrl,COUNT(DISTINCT artist) FROM Song GROUP BY album HAVING (length(album) > 0);

            ClientDb::SortDescriptor sortDescriptor;
            auto sortByAlbumName = IApp::instance()->preferences().boolForKey("SortAlbumsByAlbumName");
            if (sortByAlbumName) {
                sortDescriptor = ClientDb::SortDescriptor({groupBy.at(0)},true);
            } else {
                sortDescriptor = ClientDb::SortDescriptor({groupBy.at(1), groupBy.at(0)},true);
            }
            auto distinctField = std::string(":distinctcount:") + groupBy.at(1);
            data = db->fetchGroupings("Song", predicate, sortDescriptor, {groupBy.at(0), groupBy.at(1), "albumArtUrl", distinctField}, {groupBy.at(0)});
            
            static const std::string kVarious("Various Artists");
            if (sortByAlbumName) {
                for (auto &item : data) {
                    if (item.get(distinctField,0).asInt() > 1) {
                        item["artist"] = kVarious;
                    }
                }
            } else {
                std::vector<Json::Value> various;
                data.erase(remove_if(data.begin(), data.end(), [&](const Json::Value &item){
                    if (item.get(distinctField,0).asInt() > 1) {
                        various.push_back(item);
                        return true;
                    }
                    return false;
                }), data.end());
                sort(various.begin(), various.end(), [](const Json::Value &lhs, const Json::Value &rhs){
                    return lhs.get("album","").asString() < rhs.get("album","").asString();
                });

                Json::Value probe;
                probe["artist"] = kVarious;
                auto it = lower_bound(data.begin(), data.end(), probe, [](const Json::Value &lhs, const Json::Value &rhs){
                    return lhs.get("artist","").asString() < rhs.get("artist","").asString();
                });
                for (auto &item : various) {
                    item["artist"] = kVarious;
                }
                data.insert(it, various.begin(), various.end());
            }
            
        } else if (groupBy.size() == 1) {

            // artist:
            //SELECT artist,album,albumArtUrl FROM Song GROUP BY artist,album HAVING (length(artist) > 0);
        
            // genre:
            // SELECT genre,album,artist,albumArtUrl FROM Song GROUP BY genre,album HAVING (length(genre) > 0);

            ClientDb::SortDescriptor sortDescriptor({groupKey,"album"}, true);
            std::vector<std::string> fields = {groupKey, "albumArtUrl"};
            auto add = [&](const string &arg){
                if (find(fields.begin(), fields.end(), arg) == fields.end()) {
                    fields.push_back(arg);
                }
            };
            add("album");
            add("artist");
            data = db->fetchGroupings("Song", predicate, sortDescriptor, fields, {groupKey, "album"});
        } 

        std::string currentTitle;
        SongGrouping current;
        std::vector<SongGrouping> ret;

        auto addCurrent = [&]{
            if (currentTitle.empty()) {
                return;
            }
            ret.push_back(current);
        };

        auto originalArray = _array;
        auto self = shared_from_this();
        auto create = [originalArray,self,groupBy](const SongGrouping &grouping)->shared_ptr<IPlaylist>{
            auto currentStep = groupBy.at(0);
            string nextStep = currentStep == "genre"
                                ? "artist"
                                : (currentStep == "albumArtist" || currentStep == "artist")
                                    ? "album"
                                    : "";

            auto pred = ClientDb::Predicate::compositeAnd(originalArray->inherentPredicate(), grouping.predicate());
            //pred = originalArray->inherentPredicate();
            //pred = ClientDb::Predicate();
            shared_ptr<SortedSongArray> array(new SortedSongArray(originalArray->db(), pred, std::map<string,string>()));
            if (!nextStep.empty()) {
                return GroupedPlaylist::create(array, grouping.title(), {nextStep}, self);
            } else {
                return shared_ptr<BasicPlaylist>(new BasicPlaylist(weak_ptr<Gear::ISession>(), grouping.title(), "", array, self));
            }
        };
        for (auto &item : data) {
            auto title = item.get(groupKey,"").asString();

            std::string subtitle;
            if (groupBy.size() >= 2) {
                subtitle = item.get(groupBy.at(1),"").asString();
            }
            AlbumInfo albumInfo(item.get("artist","").asString(), item.get("album","").asString(), item.get("albumArtUrl","").asString());
            SongGrouping processing(title, subtitle, groupKey, title, multiImage, albumInfo, create);
            if (title != currentTitle) {
                addCurrent();
                currentTitle = title;
                current = processing;

            } else {
                /*if (!subtitle.empty() && current.subtitle() != subtitle) {
                    current.setSubtitle("Various Artists");
                }*/
                current.merge(processing);
            }
        }
        addCurrent();
 
#ifdef DEBUG
        std::cout << "groupSongs results: " << ret.size() << " for: " << playlistId() << "/" << groupBy.at(0) <<  " \n";
#endif
        return ret;
    }
    
    void method recalculate()
    {
        auto &groupBy = _groupBy;
        auto &predicate = _predicate;
        weak_ptr<GroupedPlaylist> wThis = shared_from_this();
        ++_calculateId;
        auto currentId = _calculateId;
        
        Io::get().dispatch([currentId, wThis, groupBy, predicate]{
            
            ClientDb::Predicate usePredicate;
            {
                auto pThis = wThis.lock();
                if (!pThis) {
                    return;
                }
                
                if (currentId != pThis->_calculateId) {
                    return;
                }

                auto array = MEMORY_NS::dynamic_pointer_cast<SortedSongArray>(pThis->_array);
                if (!array) {
                    return;
                }
                usePredicate = ClientDb::Predicate::compositeAnd(array->inherentPredicate(), predicate);
                
                //std::cout << "recalculate begin " << currentId << " (" << (void *)pThis.get() << std::endl;
            
            
                auto result = pThis->groupSongs(usePredicate, groupBy);
                //std::cout << "recalculate end " << currentId << std::endl;
            
                pThis->_availableGroupings = result;
            }
            if (IApp::instance()->phoneInterface()) {
                auto sessionManager = IApp::instance()->sessionManager();
                if (sessionManager) {
                    sessionManager->recomputeCategories();
                }
            }
        });
    }
    
    shared_ptr<ISongArray> method unfilteredArray() const
    {
        return _array;
    }
    
    bool method saveForOfflinePossible() const
    {
        return false;
    }
}
