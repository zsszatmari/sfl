//
//  GooglePlayThumbsArray.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 8/7/13.
//
//

#include "GooglePlayThumbsArray.h"

namespace Gear
{
#define method GooglePlayThumbsArray::
    
    shared_ptr<GooglePlayThumbsArray> method create(const shared_ptr<GooglePlaySession> &session)
    {
        shared_ptr<GooglePlayThumbsArray> ret(new GooglePlayThumbsArray(session));
        ret->fetch();
        return ret;
    }
    
    method GooglePlayThumbsArray(const shared_ptr<GooglePlaySession> &session) :
    // this cannot be 'temp:' since must be merged with regular thumbs up songs'
        GooglePlayNonLocalArray(session,ClientDb::Predicate::compositeAnd(ClientDb::Predicate("source", session->sessionIdentifier()), ClientDb::Predicate("playlist","ephemthumbsup")))
    {
    }       
    
    void method fetch()
    {
//#warning houston
        //return;

        auto pThis = shared_from_this();
        
        auto lSession = session().lock();
        if (lSession) {
            lSession->executor().addTask([pThis, lSession]{
                
                auto songs = lSession->ratedSongsSync();
                // filter out 4 star ratings form thumbs uo
                // Note: this way ephemeral 4 star ratings won't make it into highly rated either,
                // we need a better solution, preferably after database refactor
                songs.erase(remove_if(songs.begin(), songs.end(),[](const SongEntry &entry){
                    return entry.song()->rating() < 5;
                }), songs.end());
        

                #pragma message("TODO: ephemeral thumbs up do not work")
                //pThis->appendAllSongs(songs);
            });
        }
    }

    const bool method orderedArray() const
    {
        return false;
    }
}