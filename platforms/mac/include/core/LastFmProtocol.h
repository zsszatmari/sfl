#ifndef LASTFMPROTOCOL_H
#define LASTFMPROTOCOL_H

//
//  LastFmSession.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/17/12.
//
//

#include <map>
#include "stdplus.h"
#include "IWebWindowDelegate.h"
#include "json-forwards.h"
#include SHAREDFROMTHIS_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Base
{
    class IExecutor;
}

namespace Gear
{
    using std::string;
    using std::map;
    
    class IWebWindow;
    
    class LastFmProtocol final : public IWebWindowDelegate, public MEMORY_NS::enable_shared_from_this<LastFmProtocol>
    {
    public:
        // rate limiting applies so please use this one!
        static shared_ptr<Base::IExecutor> executor();
        static shared_ptr<LastFmProtocol> create(const string &sessionKey, const string &name);
        const string &sessionKey() const;
        const string &name() const;
        bool loggedIn() const;
        bool authenticate(string &err);
        bool authenticateMobile(string &err, const std::string &user, const std::string &pass);
        bool updateNowPlaying(const map<string, string> &songParams);
        bool scrobble(const map<string, string> &songParams, string &error);
        static Json::Value getAlbumInfo(const map<string,string> &params);
        static Json::Value getTrackInfo(const map<string,string> &params);
        static Json::Value getArtistInfo(const map<string,string> &params);
        
    private:
        LastFmProtocol(const string &sessionKey, const string &name);
        string getTokenWithErr(string &err);
        bool getSessionForToken(const string &token);
        
        virtual void didFailLoad(const string &errorMessage);
        virtual void didFinishLoad();
        virtual void navigationAction();
        virtual bool controlledCookies() const;
        virtual bool allowStandaloneBrowser() const;
        
        string _sessionKey;
        string _name;
        string _unusedAuthToken;
        shared_ptr<IWebWindow> _webWindow;
    };
}

#endif
