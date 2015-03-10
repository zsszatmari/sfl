//
//  OAuthConnection.h
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/4/13.
//
//

#ifndef __G_Ear_Player__OAuthConnection__
#define __G_Ear_Player__OAuthConnection__

#include <string>
#include "stdplus.h"
#include "IWebWindowDelegate.h"
#include MUTEX_H
#include CONDITION_VARIABLE_H
#include SHAREDFROMTHIS_H
#include ATOMIC_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    using std::string;
    using std::function;

    class IDownloader;
    
    inline std::map<string,string> emptyMap()
    {
        return std::map<string,string>();
    }
    
    class OAuthConnection : public IWebWindowDelegate, public MEMORY_NS::enable_shared_from_this<OAuthConnection>
    {
    public:
        static shared_ptr<OAuthConnection> create(const string &authServer, const string &clientId, const string &secret, const string &scope, const string redirectUri, const string &keychain);
        void callAsync(const string &url, const string &m, const string &body, const std::map<string,string> &aHeaders, const function<void(const string &,int)> &callback);
        bool autoconnect();
        bool connectSync();
        
    protected:
        OAuthConnection(const string &authServer, const string &clientId, const string &secret, const string &scope, const string redirectUri, const string &keychain);
        
    private:
        virtual void didFailLoad(const string &errorMessage);
        virtual void didFinishLoad();
        virtual void navigationAction();
        virtual bool controlledCookies() const;
        void proceed();
        
        THREAD_NS::mutex _loginMutex;
        THREAD_NS::condition_variable _loginCondition;
        bool _loginProceed;
        ATOMIC_NS::atomic<bool> _fail;
        
        string _refreshToken;
        string _accessToken;
        string _keychain;
        
        string accessTokenSync();
        string refreshTokenSync();
        string authCodeSync();
        void saveTokens(IDownloader &downloader);
        
        string _authServer;
        string _clientId;
        string _secret;
        string _scope;
        string _redirectUri;
    };
}

#endif /* defined(__G_Ear_Player__OAuthConnection__) */
