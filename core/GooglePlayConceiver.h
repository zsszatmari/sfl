//
//  GooglePlayConceiver.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#ifndef __G_Ear__GooglePlayConceiver__
#define __G_Ear__GooglePlayConceiver__

#include <vector>
#include "stdplus.h"
#include "json.h"
#include "IConceiver.h"

namespace Gear
{
    class ISession;
    class ISong;
    class GooglePlaySession;
    
    class GooglePlayConceiver final : public IConceiver
    {
    public:
        enum class Hint
        {
            search,
            fetchArtist,
            fetchAlbum,
            fetchRadio,
            radioListUser,
            createPlaylist,
            addToPlaylist,
            addToLibrary,
            loadLibrary,
            loadPlaylist
        };
        
        GooglePlayConceiver(const Json::Value &value, const weak_ptr<GooglePlaySession> &session, int hint = 0);
        virtual const vector<SongEntry> songs(const string &playlistId) const;
        virtual const vector<shared_ptr<IArtist>> artists() const;
        virtual const vector<shared_ptr<IAlbum>> albums() const;
        virtual const vector<shared_ptr<IPlaylist>> playlists() const;
        virtual bool starRatedPresent() const;
        
        static std::map<string,int> keysToIndexesTable();
        
    private:
        const Json::Value _json;
        const Hint _hint;
        const weak_ptr<GooglePlaySession> _session;
        
        const vector<string> ids() const;
        mutable bool _starRatedPresent;
    };
}

#endif /* defined(__G_Ear__GooglePlayConceiver__) */
