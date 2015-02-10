//
//  UnionSongArray.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/27/13.
//
//

#include <numeric>
#include "UnionSongArray.h"
#include "UnionSongIntent.h"
#include "BackgroundExecutor.h"
#include "IApp.h"
#include "Db.h"
#include "FreeSearchArray.h"

namespace Gear
{
#define method UnionSongArray::

    using THREAD_NS::lock_guard;
    using THREAD_NS::recursive_mutex;
    
    shared_ptr<UnionSongArray> method create()
    {
        auto ret = shared_ptr<UnionSongArray>(new UnionSongArray());
        return ret;
    }
    
    method UnionSongArray() :
        SortedSongArray(IApp::instance()->db(),ClientDb::Predicate(ClientDb::Predicate::Operator::Or,std::vector<ClientDb::Predicate>()))
	{
		_resortCount = 0;
    }
    
    void method setSongArrays(const vector<shared_ptr<SortedSongArray>> &songArrays)
    {
        //std::cout << "Union: arrays set:" << this << " " << songArrays.size() << std::endl;

    	{
    		lock_guard<recursive_mutex> l(_songArraysMutex);
    		_songArrays = songArrays;
    	}

        std::vector<ClientDb::Predicate> predicates;
        transform(songArrays.begin(), songArrays.end(), back_inserter(predicates),[](const shared_ptr<SortedSongArray> &array){
            return array->inherentPredicate();
        });

        setInherentPredicate(ClientDb::Predicate(ClientDb::Predicate::Operator::Or, predicates));
        
        #pragma message("TODO: make UnionSongArray work")
    }

    void method songsAccessed() const
    {
        //std::cout << "Union: Song Accessed " << this << " " << _songArrays.size() << "\n";
    }
    
    vector<shared_ptr<SortedSongArray>> method songArrays() const
	{
    	lock_guard<recursive_mutex> l(_songArraysMutex);
    	return _songArrays;
	}

    void method resort()
    {
    }
    
    shared_ptr<ISongArray> method arrayForSong(const SongEntry &song) const
    {
        auto identifiers = _identifiers.ptr();
        for (auto &p : *identifiers) {
            if (std::find(p.second.begin(), p.second.end(), song.entryId()) != p.second.end()) {
                return p.first;
            }
        }
        return shared_ptr<ISongArray>();
    }
    
    void method setFilterPredicate(const SongPredicate &f)
    {
        // do filtering on our own... except for free search
        auto sa = songArrays();
        
        //bool onOurOwn = true;
        for (auto it = sa.begin() ; it != sa.end() ; ++it) {
            auto &array = *it;
            if (array->needsPredicate()) {
                array->setFilterPredicate(f);
                //onOurOwn = false;
            }
        }
        //if (onOurOwn) {
        // even if we are not on our own (subarray, which is a free search array, needs the filter to get data), 
        // we still need to filter by artist, etc...
            SortedSongArray::setFilterPredicate(f);
        //}
    }
    
    bool method relevanceBased() const
    {
        // currently it's only all access...
        auto sa = songArrays();
        for (auto it = sa.begin() ; it != sa.end() ; ++it) {
            auto &array = *it;
            if (array->needsPredicate()) {
                return true;
            }
        }
        return false;
    }
    
    const string method emptyText() const
    {
        string ret;
        for (auto &arr : songArrays()) {
            string text = arr->emptyText();
            if (text.empty()) {
                ret = "";
                break;
            } else {
                ret = text;
            }
        }
        return ret;
    }
    
    bool method needsPredicate() const
    {
        for (auto &arr : songArrays()) {
            if (arr->needsPredicate()) {
                return true;
            }
        }
        return false;
    }
    
    const bool method orderedArray() const
	{
        if (relevanceBased()) {
            return true;
        }
    	for (auto &arr : songArrays()) {
            if (arr->orderedArray()) {
                return true;
            }
        }
        return false;
	}
}
