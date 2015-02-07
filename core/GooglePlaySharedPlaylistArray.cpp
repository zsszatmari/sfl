//
//  GooglePlaySharedPlaylistArray.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/20/13.
//
//

#include "GooglePlaySharedPlaylistArray.h"

namespace Gear
{
#define method GooglePlaySharedPlaylistArray::
    
    shared_ptr<GooglePlaySharedPlaylistArray> method create(const string &playlistId,const shared_ptr<GooglePlaySession> &session)
    {
        shared_ptr<GooglePlaySharedPlaylistArray> ret(new GooglePlaySharedPlaylistArray(playlistId,session));
        ret->fetch();
        return ret;
    }
    
    method GooglePlaySharedPlaylistArray(const string &playlistId, const shared_ptr<GooglePlaySession> &session) :
        GooglePlayNonLocalArray(session,ClientDb::Predicate::compositeAnd(ClientDb::Predicate("source", session->sessionIdentifier()), ClientDb::Predicate("playlist",playlistId))),
        _playlistId(playlistId)
    {
    }
    
    void method fetch()
    {
        auto pThis = shared_from_this();
        
        auto lSession = session().lock();
        if (lSession) {
            lSession->executor().addTask([pThis, lSession]{
                #pragma message("TODO: shared playlist do not work")
                auto songs = lSession->sharedSongsSync(pThis->_playlistId);
                //pThis->appendAllSongs(songs);
            });
        }
    }
}