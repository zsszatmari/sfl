    //
//  GooglePlayRadioArray.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/4/13.
//
//

#include "GooglePlayRadioArray.h"
#include "AllAccessToLibraryIntent.h"

namespace Gear
{
    GooglePlayRadioArray::MutableRadio::MutableRadio() :
        fetchedFirst(false),
        fetching(false)
    {
    }
    
#define method GooglePlayRadioArray::
    
    shared_ptr<GooglePlayRadioArray> method create(const string &radioId,const shared_ptr<GooglePlaySession> &session)
    {
        shared_ptr<GooglePlayRadioArray> ret(new GooglePlayRadioArray(radioId, session));
        return ret;
    }
    
    method GooglePlayRadioArray(const string &radioId, const shared_ptr<GooglePlaySession> &session) :
        GooglePlayNonLocalArray(session,ClientDb::Predicate::compositeAnd(ClientDb::Predicate("source", session->sessionIdentifier()), ClientDb::Predicate("playlist","temp:" + radioId))),
        _radioId(radioId)
    {
    }

    const bool method orderedArray() const
    {
        return true;
    }
    
    void method fetchMore() const
    {
        bool expected = false;
        auto lSession = session().lock();
        if (lSession) {
            if (mr.fetching.compare_exchange_strong(expected, true)) {
                auto pThis = MEMORY_NS::const_pointer_cast<GooglePlayRadioArray>(shared_from_this());
                mr.executor.addTask([pThis, lSession]{
                    auto songs = lSession->radioSongsSync(pThis->_radioId);
//                    std::cout << "radio fetched: " << songs.size() << "\n";

#pragma message("TODO: should repair fetching radio songs")
                    pThis->appendAllSongs(songs);
                    pThis->mr.fetching = false;
                });
            }
        }
    }
    
    void method songsLeft(unsigned long count)
    {
        if (count < 3) {
            fetchMore();
        }
    }

    void method songsAccessed() const
    {
        if (!mr.fetchedFirst) {
            //cout << "nott Fetched!!!\n";
            mr.fetchedFirst = true;
            fetchMore();
            
        } else {
            //cout << "Fetched!!!\n";
        }
    }
}