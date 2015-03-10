//
//  GoogleMusicConnection.h
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 31/10/13.
//
//

#ifndef __G_Ear_Player__GoogleMusicConnection__
#define __G_Ear_Player__GoogleMusicConnection__

#include <map>
#include <vector>
#include "Environment.h"
#include "stdplus.h"
#include "json-forwards.h"
#include "IWebWindowDelegate.h"
#include "WebCookie.h"
#include MUTEX_H
#include CONDITION_VARIABLE_H
#include CHRONO_H
#include SHAREDFROMTHIS_H
#include MEMORY_H
using MEMORY_NS::shared_ptr;

namespace Gear
{
    class WebCookie;
    class IWebWindow;
    class IDownloader;
    class JsonStreamer;

    using std::function;
    
    class GoogleMusicConnection : public IWebWindowDelegate, public MEMORY_NS::enable_shared_from_this<GoogleMusicConnection>
    {
    public:
        static shared_ptr<GoogleMusicConnection> create();
        
        bool loginWithCookies(const Json::Value &cookies);
        void call(const string &name, const Json::Value &param, const function<void(const shared_ptr<Json::Value> &)> &callback);
        void callPure(const std::string &name, const function<void(const std::string &)> &callback);
        void setUserName(const std::string &name);
        const std::string &userName() const;
        
        std::map<std::string, std::string> buildHeaders();
        WebCookie cookieForName(const std::string &name);
        const std::vector<WebCookie> &cookiesToSave() const;
        void streamingCall(const string &name, const Json::Value &param, const function<void(const Json::Value &)> &gotChunk);
        
        static void fixJson(string &json);
        
    protected:
        GoogleMusicConnection();

    private:
        enum class LoginState
        {
            LoginStateNoLogin = 0,
            LoginStateLoadingStart,
            LoginStateLoggingIn,
            LoginStateLoggedin
        };
        
        LoginState _loginState;
        
        std::string buildUrl(const std::string &name);
        std::string _userAgent;
        std::vector<WebCookie> _cookies;
        std::string _userName;
        
        THREAD_NS::mutex _loginLock;
        bool _loginCanProceed;
        THREAD_NS::condition_variable _loginCondition;
        
        string _errorMessage;
        
        shared_ptr<IWebWindow> _webWindow;
        
        void close();
        void showWebView();
        void finishLogin();
        bool isWebViewShown();
        
        CHRONO_NS::time_point<CHRONO_NS::system_clock> _cookieValidUntil;
        THREAD_NS::mutex _cookieValidMutex;
        bool loginWithCookies(const std::vector<std::map<string,string>> cookies);
        void startDownload(const string &name, const Json::Value &param, const string &m, const function<void(const string &)> &callback, int chunkSize = 0);
        
        virtual void didFailLoad(const string &errorMessage);
        virtual void didFinishLoad();
        virtual void navigationAction();
    };
}

#endif /* defined(__G_Ear_Player__GoogleMusicConnection__) */
