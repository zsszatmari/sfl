    //
//  OAuthConnection.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/4/13.
//
//

#include <iostream>
#include <sstream>
#include "json.h"
#include "MainExecutor.h"
#include "OAuthConnection.h"
#include "IApp.h"
#include "UrlEncode.h"
#include "IWebWindow.h"
#include "HttpDownloader.h"
#include "IKeychain.h"
#include "Logger.h"
#include "AsyncHttp.h"
#include "IoService.h"

namespace Gear
{
    using namespace Base;
    using THREAD_NS::mutex;
    using THREAD_NS::lock_guard;
    using THREAD_NS::unique_lock;
    
#define method OAuthConnection::
    
    shared_ptr<OAuthConnection> method create(const string &authServer, const string &clientId, const string &secret, const string &scope, const string redirectUri, const string &keychain)
    {
        return shared_ptr<OAuthConnection>(new OAuthConnection(authServer, clientId, secret, scope, redirectUri, keychain));
    }
    
    method OAuthConnection(const string &authServer, const string &clientId, const string &secret, const string &scope, const string redirectUri, const string &keychain) :
        _authServer(authServer),
        _clientId(clientId),
        _scope(scope),
        _secret(secret),
        _fail(false),
        _keychain(keychain),
        _redirectUri(redirectUri)
    {
        _refreshToken = IApp::instance()->keychain()->pass(keychain);
    }
    
    string method authCodeSync()
    {
        auto pThis = shared_from_this();
        
        std::stringstream url;
        url << _authServer << "/auth?client_id=" << _clientId << "&redirect_uri=" << UrlEncode::encode(_redirectUri) <<
            "&response_type=code" <<
            "&scope=" << UrlEncode::encode(_scope) <<
            "&access_type=offline";
        auto urlStr = url.str();
        
        bool nonLocalhostRedirect = (_redirectUri.find("urn:") == 0);
        
        _loginProceed = false;
        string resultUrl;
        string resultTitle;
        {
            shared_ptr<IWebWindow> webWindow;
            MainExecutor::instance().addTaskAndWait([pThis, &webWindow, &urlStr]{
                webWindow = IApp::instance()->createWebWindow(pThis);
                webWindow->loadUrl(urlStr);
                webWindow->show("YouTube authentication");
            });
            
            while(true) {
                resultUrl.clear();
                resultTitle.clear();
                
                bool canProceed = false;
                {
                    unique_lock<mutex> l(_loginMutex);
                    canProceed = _loginProceed;
                    if (!canProceed) {
                        _loginCondition.wait_for(l, CHRONO_NS::milliseconds(1000));
                    }
                }
                if (!canProceed) {
                    MainExecutor::instance().addTaskAndWait([&]{
                        if (!webWindow->visible()) {
                            _fail = true;
                        }
                    });
                    if (_fail) {
                        break;
                    }
                } else {
                    MainExecutor::instance().addTaskAndWait([&]{
                        
                        if (nonLocalhostRedirect) {
                            resultTitle = webWindow->title();
                            if (!_fail) {
                                string pref = "Success ";
                                
                                if (resultTitle.compare(0, pref.length(), pref) != 0) {
                                    // still waiting for a result
                                    lock_guard<mutex> l(_loginMutex);
                                    _loginProceed = false;
                                    canProceed = false;
                                    return;
                                }
                            }
                        } else {
                            
                            string pref(_redirectUri);
                            resultUrl = webWindow->url();
                            Logger::stream() << "youtube connect: " << resultUrl << " fail state: " << (bool)_fail;
                            if (!_fail) {
                                if (resultUrl.compare(0, pref.length(), pref) != 0) {
                                    // still waiting for a result
                                    lock_guard<mutex> l(_loginMutex);
                                    _loginProceed = false;
                                    canProceed = false;
                                    return;
                                }
                            }
                        }
                        webWindow.reset();
                    });
                    if (canProceed) {
                        break;
                    }
                }
            }
        }
        
        if (nonLocalhostRedirect) {
            string code = "code=";
            auto pos = resultTitle.find(code);
            if (pos == string::npos) {
                return "";
            }
            auto codeEnd = resultTitle.find("&", pos);
            if (codeEnd == string::npos) {
                return resultTitle.substr(pos + code.length());
            } else {
                return resultTitle.substr(pos + code.length(), codeEnd - (pos + code.length()));
            }
        } else {
            string code = "code=";
            auto pos = resultUrl.find(code);
            if (pos == string::npos) {
                return "";
            }
            return resultUrl.substr(pos + code.length());
        }
    }
    
    string method refreshTokenSync()
    {
        if (!_refreshToken.empty()) {
            return _refreshToken;
        }
        
        auto authCode = authCodeSync();
        if (authCode.empty()) {
            return "";
        }
        
        map<string, string> headers;
        std::stringstream body;
        body << "client_id=" << _clientId <<
                "&client_secret=" << _secret <<
                "&grant_type=authorization_code" <<
                "&redirect_uri=" << _redirectUri <<
                "&code=" << authCode;
        
        headers.insert(std::make_pair("Content-Type", "application/x-www-form-urlencoded"));
        //headers.inserţ(std::make_pair("Accept", "*/*"));
        //headers.insert(std::make_pair("User-Agent", "curl/7.24.0 (x86_64-apple-darwin12.0) libcurl/7.24.0 OpenSSL/0.9.8y zlib/1.2.5"));
        
        auto str = body.str();

        auto downloader = HttpDownloader::create(_authServer + "/token", headers, "POST", str);
        //auto debugCurl = HttpDownloader::curlDump(_authServer + "/token", headers, "POST", str);
        downloader->waitUntilFinished();
        
        auto failed = downloader->failed();
        if (failed) {
            return "";
        }
        
        saveTokens(*downloader);
        return _refreshToken;
    }
    
    string method accessTokenSync()
    {
        // accesstoken must be only accessed from io thread
        assert(Io::isCurrent());

        if (!_accessToken.empty()) {
            return _accessToken;
        }
        auto refreshToken = refreshTokenSync();
        
        // it might be that we got the access token together with the refresh token (first login)
        if (!_accessToken.empty()) {
            return _accessToken;
        }
        if (refreshToken.empty()) {
            return "";
        }
        
        map<string, string> headers;
        std::stringstream body;
        body << "client_id=" << _clientId <<
            "&client_secret=" << _secret <<
            "&grant_type=refresh_token" <<
            "&refresh_token=" << refreshToken;
        
        headers.insert(std::make_pair("Content-Type", "application/x-www-form-urlencoded"));
        
        auto str = body.str();

        auto downloader = HttpDownloader::create(_authServer + "/token", headers, "POST", str);
        downloader->waitUntilFinished();
        
        if (downloader->failed()) {
            return "";
        }
        
        saveTokens(*downloader);
        return _accessToken;

    }
    
    void method saveTokens(IDownloader &downloader)
    {
        assert(Io::isCurrent());

        string data = downloader;
        
        Json::Value value;
        Json::Reader reader;
        bool success = reader.parse(data, value);
        
        if (success) {
            string access = value.get("access_token", "").asString();
            string refresh = value.get("refresh_token", "").asString();
            
            if (!access.empty()) {
                _accessToken = access;
            }
            if (!refresh.empty()) {
                _refreshToken = refresh;
                IApp::instance()->keychain()->save(_keychain, "unknown", refresh);
            }
        }
    }
    
    bool method controlledCookies() const
    {
        return false;
    }
    
    bool method autoconnect()
    {
        return !_refreshToken.empty();
    }
    
    bool method connectSync()
    {
        _fail = false;
        auto access = accessTokenSync();
        return !access.empty();
    }
    
    void method callAsync(const string &url, const string &m, const string &body, const map<string,string> &aHeaders, const function<void(const string &,int)> &callback)
    {
        map<string, string> headers = aHeaders;
        {
            auto access = accessTokenSync();
            if (access.empty()) {
                callback("",0);
                return;
            }
            headers.insert(std::make_pair("Authorization", "Bearer " + access));
        }
        
        
        auto self = shared_from_this();
        AsyncHttp::download(Io::get(),AsyncHttp::Request(url, headers, m, body), [callback,self,url,m,body](const AsyncHttp::Response &response){

            assert(Io::isCurrent());

            auto fail = response.statusCode;
            if (fail == 204 || fail == 200) {
                // 'no content' 
                fail = 0;
            }
    #if DEBUG
            if (fail != 0) {
                std::cout << "http fail: " << fail << std::endl;
            }
    #endif
            
            string ret = response.bodyAsString();
            auto gotResult = [callback](const string &str, int fail){
                int resultCode = fail;
                if (resultCode == 0) {
                    resultCode = 200;
                }
                if (fail) {
                    callback("",resultCode);
                }
                
                callback(str, resultCode);
            };
            if (fail == 401) {
                if (ret.find("youtubeSignupRequired") != std::string::npos) {
                    // speciel casse, don't renew
    #ifdef DEBUG
                    std::cout << "oauth signup required: " << ret << std::endl;
    #endif
                    gotResult(ret, fail);
                } else {
                    //expired
                    // TODO: make sure expiration handled the right way
    #ifdef DEBUG
                    std::cout << "oauth call renew " << ret << std::endl;
    #endif
                    
                    self->_accessToken = "";
                    auto access = self->accessTokenSync();
                    if (access.empty()) {
                        callback("", 0);
                        return;
                    }
                    
                    map<string,string> newHeaders;
                    newHeaders.insert(std::make_pair("Authorization", "Bearer " + access));
                    
                    AsyncHttp::download(Io::get(),AsyncHttp::Request(url, newHeaders, m, body), [callback,gotResult](const AsyncHttp::Response &response){
#ifdef DEBUG
                        std::cout << "oauth call renew: " << response.statusCode << std::endl;
#endif
                        gotResult(response.bodyAsString(), response.statusCode);
                    });
                }
            } else {
                gotResult(ret, fail);
            }
        });
    }
    
    void method didFailLoad(const string &errorMessage)
    {
        _fail = true;
        proceed();
    }
    
    void method didFinishLoad()
    {
        proceed();
    }
    
    void method proceed()
    {
        lock_guard<mutex> l(_loginMutex);
        _loginProceed = true;
        _loginCondition.notify_all();
    }
    
    void method navigationAction()
    {
    }
}
