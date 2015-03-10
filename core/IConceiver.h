//
//  IConceiver.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/28/13.
//
//

#ifndef __G_Ear__IConceiver__
#define __G_Ear__IConceiver__

#include <vector>
#include "stdplus.h"
#include "SongEntry.h"
#include "json-forwards.h"
#include "ISession.h"
#include "IArtist.h"
#include "IAlbum.h"

namespace Gear
{    
    class IConceiver
    {
    public:
        IConceiver(const weak_ptr<const ISession> &session);
        virtual ~IConceiver();
        virtual const vector<Json::Value> songs(const string &playlistId) const = 0;
        virtual const vector<shared_ptr<IArtist>> artists() const = 0;
        virtual const vector<shared_ptr<IAlbum>> albums() const = 0;
        virtual const vector<Json::Value> playlists() const = 0;
        virtual bool starRatedPresent() const = 0;
        
    protected:
        const weak_ptr<const ISession> session() const;
        
    private:
        const weak_ptr<const ISession> _session;
    };
    
    template<class TSession>
    class IConceiverFactory
    {
    public:
        virtual ~IConceiverFactory()
        {
        }
        
        virtual std::unique_ptr<const IConceiver>conceiver(const Json::Value &value, const weak_ptr<TSession> &session, int hint = 0) const = 0;
    };
    
    template<class Conceiver, class TSession>
    class ConceiverFactory : public IConceiverFactory<TSession>
    {
    public:
        virtual std::unique_ptr<const IConceiver>conceiver(const Json::Value &value, const weak_ptr<TSession> &session, int hint = 0) const
        {
            return std::unique_ptr<const IConceiver>(new Conceiver(value, session, hint));
        }
    };
}

#endif /* defined(__G_Ear__IConceiver__) */
