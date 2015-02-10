//
//  SessionSongArray.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/5/13.
//
//

#include "SessionSongArray.h"
#include "AllAccessToLibraryIntent.h"
#include "CreateRadioIntent.h"
#include "AddToPlaylistIntent.h"
#include "CreatePlaylistIntent.h"
#include "OfflineIntent.h"
#include "BaseUtility.h"
#include "Db.h"

namespace Gear
{
#define method SessionSongArray::
    method SessionSongArray(const shared_ptr<ISession> &session, const ClientDb::Predicate &predicate) :
        SortedSongArray(IApp::instance()->db(), predicate),
        _session(session),
        _app(IApp::instance())
    {
    }
    
    const weak_ptr<ISession> method session() const
    {
        return _session;
    }
}
