//
//  FreeSearchArray.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/4/13.
//
//

#include "FreeSearchArray.h"


#include "stdplus.h"
#include "AllAccessSongArray.h"

namespace Gear
{
    FreeSearchArray::MutableState::MutableState() :
        runningSearch(false)
    {
    }
    
#define method FreeSearchArray::
    
    shared_ptr<FreeSearchArray> method create(const shared_ptr<ISession> &session)
    {
        shared_ptr<FreeSearchArray> ret(new FreeSearchArray(session));
        return ret;
    }
    
    method FreeSearchArray(const shared_ptr<ISession> &session) :
        NonLocalArray(session,ClientDb::Predicate::compositeAnd(ClientDb::Predicate("source", session->sessionIdentifier()), ClientDb::Predicate("playlist","temp:free")))
    {
    }
    
    bool method operator==(const ISongArray &rhs)
    {
        const AllAccessSongArray *rPtr = dynamic_cast<const AllAccessSongArray *>(&rhs);
        return rPtr != nullptr;
    }
    

    #pragma message("TODO free search must be re-implemented")
    
    void method setFilterPredicate(const SongPredicate &f)
    {
        auto oldPredicate = filterPredicate();
        bool requestChanged = (f != oldPredicate);
        
        SortedSongArray::setFilterPredicate(f);
        
        //std::cout << "willstart1 search for " << f.value() << std::endl;
        
        if (requestChanged) {
            bool expect = false;
            
            //std::cout << "willstart2 search for " << f.value() << std::endl;
            if (m.runningSearch.compare_exchange_strong(expect, true)) {
                auto pThis = shared_from_this();
                
                //std::cout << "willstart3 search for " << f.value() << std::endl;
                executor.addTask([pThis] {
                    
                    bool shouldClear = true;
                    string currentFilter = pThis->filterPredicate().value();
                    
                    //std::cout << "searching for " << currentFilter << std::endl;
                    
                    string t;
                    do {
                        
                        if (shouldClear) {
                            t = "";
                        }
                        vector<SongEntry> result;
                        if (!currentFilter.empty()) {
                            auto lSession = pThis->session().lock();
                            if (lSession) {
                                result = lSession->searchSync(u(currentFilter), t);
                            }
                            
                            //std::cout << "found " << result.size() << " for " << currentFilter << std::endl;
                        }
                        
                        if (shouldClear) {
                            pThis->setAllSongs(result);
                            shouldClear = false;
                        } else {
                            pThis->appendAllSongs(result);
                        }
                        
                        bool queryFinished = result.empty();
                        
                        {
                            auto possiblyUpdatedFilter = pThis->filterPredicate().value();
                            if (currentFilter == possiblyUpdatedFilter) {
                                if (queryFinished) {
                                    pThis->m.runningSearch = false;
                                    break;
                                }
                            } else {
                                shouldClear = true;
                                currentFilter = possiblyUpdatedFilter;
                            }
                        }
                    } while (true);
                    
                });
            }
        }
    }
    
    
    const string method emptyText() const
    {
        if (filterPredicate()) {
            return u("");
            //return u("No matching songs found");
        } else {
            return u("Start typing in the search field...");
        }
    }
    /*
    bool method filterNeeded() const
    {
        return !filterPredicate().key().empty();
    }
    */
    bool method needsPredicate() const
    {
        return true;
    }

    bool method keepSongOrder() const
    {
        return false;
    }
}