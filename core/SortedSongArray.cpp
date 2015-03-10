//
//  SortedSongArray.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/4/13.
//
//

#include <iostream>
#include "BackgroundExecutor.h"
#include "MainExecutor.h"
#include "Thread.h"
#include "SortedSongArray.h"
#include "AllAccessToLibraryIntent.h"
#include "MusicSort.h"
#include "SongSortOrder.h"
#include "stdplus.h"
#include "Logger.h"
#include "ValidPtr.h"
#include <assert.h>
#include "SongView.h"
#include "Predicate.h"
#include "IDb.h"
#include "IoService.h"
#include "IApp.h"
#include "sessionManager.h"

namespace Gear
{
	using namespace Base;
    
#define method SortedSongArray::
  
    static int initrandom()
    {
        srand((unsigned int)time(NULL));
        return 0;
    }

    static uint32_t random()
    {
#ifdef __APPLE__
        return arc4random();
#else
        static int s = initrandom();
        return rand();
#endif
    }

    void method setNoRepeatBase(const SongEntry &song)
    {
        shuffleSeed = random();
        firstShuffleNoRepeatSaltedHash = shuffleHashForUniqueId(song.entryId());
    }    

    method SortedSongArray(const shared_ptr<IDb> &db, const ClientDb::Predicate &inherentPredicate, const std::map<string,string> &traits) :
        _inherentPredicate(inherentPredicate),
        shuffleSeed(random()),
        _db(db),
        _traits(traits)
    {
        setSortDescriptor(SongSortOrder::sortDescriptor());
    }

    ClientDb::SortDescriptor method orderBy() const
    {
        bool ascending = true;
        vector<string> keys;
        transform(_sortDescriptor.keysAndAscending().begin(), _sortDescriptor.keysAndAscending().end(), back_inserter(keys),
            [&](const std::pair<std::string,bool> &p){

            if (!p.second) {
                ascending = false;
            }
            return p.first;
        });
        return ClientDb::SortDescriptor(keys,ascending);
    }

    ClientDb::Predicate method predicate() const
    {
        return ClientDb::Predicate::compositeAnd(_inherentPredicate, _secondaryPredicate, _searchPredicate);
    }

    SongView method songs(const function<void(const SongView &,SongView::Event,size_t,size_t)> &callback) const
    {
        songsAccessed();
        return SongView(_db, predicate(), orderBy(), [](const function<void()> &f){
            MainExecutor::instance().addTask(f);
        }, callback);
    }

    shared_ptr<const vector<SongEntry>> method songsReallySlow() const
    {
        auto jsons = _db->fetchSync("Song", predicate(), orderBy(), false);
        shared_ptr<vector<SongEntry>> ret(new vector<SongEntry>());
        ret->reserve(jsons.size());
        transform(jsons.begin(), jsons.end(), back_inserter(*ret), [](const Json::Value &json){
        
            return SongEntry(json);
        });
        songsAccessed();
        return ret;
    }

    unsigned long method indexOf(const SongEntry &rhs) const
    {
        auto values = rhs.toJson();
        auto inherent = _inherentPredicate.keyValues();
        // not queue, but the current playlist we are interested in
        for (const auto &p : inherent) {
            values[p.first] = p.second;
        }
        //std::cout << "db: " << db.get() << std::endl;
        return _db->indexOf("Song", predicate(), orderBy(), values);
    }

    void method songsAccessed() const
    {
    }

    bool method contains(const SongEntry &song) const
    {
        #pragma message("TODO")
        return false;
    }
    
    void method resetCache()
    {
        #pragma message("TODO")
    }

    void method setFilterPredicate(const SongPredicate &f)
    {
        _filterPredicate = f;
        _searchPredicate = f;
        _eventSignal.signal();
    }

    SongPredicate method filterPredicate() const
    {
        return _filterPredicate;
    }

    void method setSecondaryFilterPredicate(const SongPredicate &f)
    {
        _secondaryPredicate = f;
        _eventSignal.signal();
    }

    void method setInherentPredicate(const ClientDb::Predicate &predicate)
    {
        _inherentPredicate = predicate;
        _eventSignal.signal();
    }

    void method removeSongs(const vector<SongEntry> &songs)
    {
        auto db = _db;
        Io::get().dispatch([songs,db]{
            std::cout << "will remove songs: " << songs.size() << std::endl;
            for (auto &song : songs) {
                db->remove("Song", song.toJson());
            }
        });
    }

    bool method moveSongs(const vector<SongEntry> &entries, const SongEntry &after, const SongEntry &before)
    {
        Io::get().dispatch([entries,after,before]{
            if (after) {
                float afterPos = after.song()->floatForKey("position");
                float increment = 1;
                if (before) {
                    float beforePos = before.song()->floatForKey("position");
                    auto count = entries.size();
                    increment = (beforePos-afterPos) / (count+1);
                }
                float newPos = afterPos;
                for (auto &entry : entries) {
                    newPos += increment;
                    entry.song()->setFloatForKey("position", newPos);
                    entry.song()->updateInDbAndRemote({"position"});
                }  
            } else if (before) {
                float beforePos = before.song()->floatForKey("position");
                float newPos = beforePos;
                for (auto it = entries.rbegin() ; it != entries.rend(); ++it) {
                    newPos -= 1;
                    auto &entry = *it;
                    entry.song()->setFloatForKey("position", newPos);
                    entry.song()->updateInDbAndRemote({"position"});
                }
            }
        });
        return true;
    }

    void method setSortDescriptor(const SortDescriptor &rhs)
    {
        if (!(rhs == _sortDescriptor)) {
            _sortDescriptor = rhs;
            _eventSignal.signal();
        }
        #pragma message("TODO add relevance if necessary!")
    }

    void method appendAllSongs(const vector<SongEntry> &songs)
    {
        setAllSongs(songs, true, false);
    }
       
    vector<std::string> method all(const string &field) const 
    {
        assert(Io::isCurrent());
        return _db->fetchSync("Song", predicate(), orderBy(), field);
    }

    std::map<int,std::string> method mapping(const string &field, int slots) const
    {
        std::map<int,std::string> ret;
        if (slots == 0) {
            return ret;
        }
        auto result = _db->fetchSync("Song", predicate(), orderBy(), field);
        if (result.empty()) {
            return ret;
        }

        for (int i = 0 ; i < slots ; ++i) {
            int position = (result.size()-1) * i / (slots-1);
            ret.insert(std::make_pair(position, result.at(position)));
        }
        return ret;
        // this is _very_ slow because of the OFFSET parameter, the query must be repeated many times
        /*
        auto num = _db->count("Song", predicate());
        if (num == 0 || slots == 0) {
            return std::map<int,std::string>();
        }
        std::map<int,std::string> ret;
        for (int i = 0 ; i < slots ; ++i) {
            int position = (num-1) * i / (slots-1);
            auto results = _db->fetchSync("Song", predicate(), orderBy(), 1, position, false);
            if (!results.empty()) {
                ret.insert(std::make_pair(position, results.at(0).get(field,"").asString()));
            }
        }
        return ret;*/
    }

    SongEntry method atSync(size_t index) const
    {
        auto results = _db->fetchSync("Song", predicate(), orderBy(), 1, index, false);
        if (results.empty()) {
            return SongEntry();
        } else {
            return SongEntry(results.at(0));
        }
    }

    uint32_t method shuffleHashForUniqueId(const string &uniqueId) const
    {
        static HASH_NS::hash<string> hasher;
        return static_cast<int32_t>(hasher(uniqueId) * shuffleSeed);
    }
    
    SongEntry method nextShuffledSongRelativeTo(const SongEntry &song, const bool ascending, const bool repeat) const
    {
        //std::cout << "isong nextshuffle " << ascending << "/" << repeat << " relative to: " << song.song()->uniqueId() << "," << song.entryId() << "\n"; 
        assert(Io::isCurrent());

        // inspection context: we should not re-rerefetch things again and again in one iteration when calculating queue...
        auto ctx = inspectionContextImpl();

        if (ctx->allEntryIds.empty()) { 
            ctx->allEntryIds = all("entryId");
        }

        const auto &allEntryIds = ctx->allEntryIds;
        
        if (allEntryIds.size() == 0) {
            return SongEntry();
        }
        
        uint32_t candidateRow;
        const auto hashBase = shuffleHashForUniqueId(song.entryId());
        
        if (allEntryIds.size() == 1) {
            
            candidateRow = 0;
        } else {
            // first: shuffle hash, second: row id

            typedef pair<uint32_t, uint32_t> p;

            auto &pairs = ctx->shuffleHashToRowId;
            if (pairs.empty()) {
                pairs.reserve(allEntryIds.size());
                int i = 0;
                transform(allEntryIds.begin(), allEntryIds.end(), back_inserter(pairs), [&](const std::string &entryId){
                    //std::cout << "isong " << i << ". entry: " << entryId << " => "<< shuffleHashForUniqueId(entryId) << " minused: " << shuffleHashForUniqueId(entryId)-firstShuffleNoRepeatSaltedHash << " frombase: " << shuffleHashForUniqueId(entryId)-hashBase << std::endl;
                    return std::make_pair(shuffleHashForUniqueId(entryId), i++);
                });
            }
            
            partial_sort(pairs.begin(), pairs.begin() + 2, pairs.end(), [&](const p &lhs, const p &rhs){
                // we count on overflow!
                const auto hashLhs = lhs.first - hashBase;
                const auto hashRhs = rhs.first - hashBase;
                if (ascending) {
                    return hashLhs < hashRhs;
                } else {
                    return hashLhs > hashRhs;
                }
            });
            
            
            if (pairs.at(0).first == hashBase) {
                candidateRow = pairs.at(1).second;
            } else {
                candidateRow = pairs.at(0).second;
            }
        }
        
        if (!repeat) {
            
            auto candidateHash = shuffleHashForUniqueId(allEntryIds[candidateRow]);
            //std::cout << "ascending: " << ascending << " norepeatbase: " << firstShuffleNoRepeatSaltedHash << " hashbase: " << hashBase << " candidate: " << candidateHash << std::endl;
            
            if (ascending) {
                // just made a full round
                if (hashBase < firstShuffleNoRepeatSaltedHash && candidateHash > firstShuffleNoRepeatSaltedHash) {
                    //std::cout << "nomore1\n";
                    return SongEntry();
                }
                if (candidateHash == firstShuffleNoRepeatSaltedHash) {
                    // we cannot reach the first again
                    return SongEntry();
                }
            } else {
                // just made a full round other direction
                if (hashBase > firstShuffleNoRepeatSaltedHash && candidateHash < firstShuffleNoRepeatSaltedHash) {
                    //std::cout << "nomore2\n";
                    return SongEntry();
                }
                if (hashBase == firstShuffleNoRepeatSaltedHash) {
                    // there is nothing before the first
                    //std::cout << "nomore3\n";
                    return SongEntry();
                }
            }
            if (candidateHash == firstShuffleNoRepeatSaltedHash && candidateHash == hashBase) {
                // special case, we shall not repeat ourselves either
                //std::cout << "nomore4\n";
                return SongEntry();
            }
        }

        auto result = atSync(candidateRow);
        //std::cout << "isong result row: " << candidateRow << " entry: " << result.entryId() <<std::endl;
        return result;
    }

    bool method empty() const 
    {
        return _db->count("Song", predicate()) == 0;
    }


    uint32_t method sizeSync() const
    {
        auto ctx = inspectionContextImpl();
        if (ctx->size >= 0) {
            return ctx->size;
        }
        ctx->size = _db->count("Song", predicate());
        return ctx->size;
    }

    SortDescriptor method sortDescriptor() const
    {
        return _sortDescriptor;
    }

    void method setAllSongs(const vector<SongEntry> &songs, bool append, bool overwrite)
    {
        auto inherent = _inherentPredicate;
        auto ordered = orderedArray();
        auto db = _db;
        Io::get().dispatch([songs,append,overwrite,inherent,ordered,db]{
            int order = 0;
            if (!append) {
                db->remove("Song", inherent);
            } else {
                order = db->max("Song", inherent, "position") +1;
            }
            // maybe we could assume that the playlist and source is already set. but this might not always be the case.
            auto keyValues = inherent.keyValues();
            for (auto &song : songs) {
                //std::cout << "adding song " << song.entryId() << std::endl;
                auto s = song.song();
                for (auto &p : keyValues) {
                    s->setStringForKey(p.first, p.second);
                }
                if (ordered) {
                    s->setIntForKey("position", order++);
                }
                s->updateInDb({"position"});
            }
        });
    }
        
    Base::EventConnector method updatedEvent()
    {
        return _eventSignal.connector();
    }

    ClientDb::Predicate method inherentPredicate() const
    {
        return _inherentPredicate;
    }

    shared_ptr<ISongArray::InspectionContext> method inspectionContext() const
    {
        auto s = _inspectionContext.lock();
        if (s) {
            return s;
        }
        shared_ptr<InspectionContextImpl> ret(new InspectionContextImpl());
        _inspectionContext = ret;
        return ret;
    }

    shared_ptr<SortedSongArray::InspectionContextImpl> method inspectionContextImpl() const
    {
        assert(Io::isCurrent());
        return MEMORY_NS::static_pointer_cast<InspectionContextImpl>(inspectionContext());
    }

    shared_ptr<IDb> method db() const
    {
        return _db;
    }

    SortedSongArray::InspectionContextImpl::InspectionContextImpl() :
        size(-1)
    {
    }

    string method predicateAsString() const
    {
        return predicate();
    }

    void method songsLeft(unsigned long count)
    {
        if (count < 3 && _traits.find("dynamic") != _traits.end() && 
                         _traits.find("source") != _traits.end() &&
                         _traits.find("playlistId") != _traits.end()) {
            auto session = IApp::instance()->sessionManager()->session(_traits.find("source")->second);
            if (session) {
                session->fetchDynamic(_traits.find("playlistId")->second);
            }
        }
    }
}
