//
//  GoogleMusicConnection.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmari on 31/10/13.
//
//

#include <iostream>
#include <sstream>
#include "GoogleMusicConnection.h"
#include "json.h"
#include "UrlEncode.h"
#include "WebCookie.h"
#include "HttpDownloader.h"
#include "MainExecutor.h"
#include "BackgroundExecutor.h"
#include "IApp.h"
#include "IWebWindow.h"
#include "JsonStreamer.h"
#include "BaseUtility.h"
#include THREAD_H

namespace Gear
{
    using std::cout;
    using std::endl;
    using std::string;
    using std::make_pair;
    using std::vector;
    using namespace Base;
    using THREAD_NS::mutex;
    using THREAD_NS::unique_lock;
    using THREAD_NS::lock_guard;
    using THREAD_NS::this_thread::sleep_for;
    
    static const long long kCookieValidityDuration = 1000 * 60 * 60 * 1;
    //static const long long kCookieValidityDuration = 1000 * 60;
    
    
#define method GoogleMusicConnection::
    
    shared_ptr<GoogleMusicConnection> method create()
    {
        return shared_ptr<GoogleMusicConnection>(new GoogleMusicConnection());
    }
    
    method GoogleMusicConnection() :
        _loginState(LoginState::LoginStateNoLogin),
        _loginCanProceed(false)
    {
    }
    
    static bool pureJsonForCall(const string &name)
    {
        static string s[] = {"getmixentries", "search", "fetchartist", "fetchalbum", "addsongstolocker", "radio/fetchradiofeed",  "radio/loadradio", "radio/createstation", "radio/deletestation", "createplaylist", "addtoplaylist", "editplaylist", "loadsharedplaylist", "getephemthumbsup", "explore/loadexplore", "loaduserplaylist", "modifytracks"};
        static vector<string> v = init<vector<string>>(s);
        
        return find(v.begin(), v.end(), name) != v.end();
    }
    
    static void replaceOccurrences(string &str, const string &search, const string &replace)
    {
        size_t pos = 0;
        while ((pos = str.find(search,pos)) != std::string::npos) {
            str.replace(pos, search.length(), replace);
        }
    }
    
    map<string, string> method buildHeaders()
    {
        if (_loginState == LoginState::LoginStateLoggedin) {
            lock_guard<mutex> l(_cookieValidMutex);
            if (CHRONO_NS::system_clock::now() > _cookieValidUntil) {
                
                std::cout << "timeout reached" << std::endl;
                
                vector<map<string,string>> cookieMaps;
                transform(_cookies.begin(), _cookies.end(), back_inserter(cookieMaps), [](const WebCookie &rhs){
                    return rhs.properties();
                });
                loginWithCookies(cookieMaps);
            }
        }
        
        map<string, string> ret;
        ret.insert(make_pair("Pragma", "no-cache"));
        ret.insert(make_pair("User-Agent", _userAgent));
        ret.insert(make_pair("Host", "play.google.com"));
        ret.insert(make_pair("Accept-Encoding", "identity"));
        
        ret.insert(make_pair("Cookie", WebCookie::cookieHeaderValue(_cookies, ".google.com")));
        
        // POST method is expected
        
        return ret;
    }
    
    shared_ptr<IDownloader> method startDownload(const string &name, const Json::Value &param, const string &m)
    {
        string url = buildUrl(name);
		map<string, string> headers = buildHeaders();
        
        bool pureJson = pureJsonForCall(name);
        
        string body;
        
        if (!param.isString()) {
            
            Json::FastWriter writer;
            string json = writer.write(param);
            {
                size_t index = 0;
                const string toReplace("\\/", index);
                while (true) {
                    index = json.find("\\/");
                    if (index == string::npos) {
                        break;
                    }
                    json.replace(index, toReplace.length(), "/");
                }
            }
            
            if (pureJson && name != "addtoplaylist") {
                // no encoding needed
                body = json;
            } else {
                json = UrlEncode::encode(json);
                
                body = "json=" + json;
            }
#ifdef VERBOSE_LOG
            NSLog(@"request headers: %@", [self loggableStringFrom:[request allHTTPHeaderFields]]);
            NSLog(@"request body: %@", [self loggableStringFrom:body]);
#endif
            // it was 'content-type'
            
            if (m == "POST") {
                headers.insert(make_pair("Content-Type", "application/x-www-form-urlencoded"));
            } else {
                url += "&" + body + "&format=jsarray";
            }
            
        } else {
            url += "&";
            url += param.asString();
        }
        
#ifdef DEBUG
        cout << "send request: " << url << endl;
#endif
        auto downloader = HttpDownloader::create(url, headers, m, m == "POST" ? body : "");
        return downloader;
    }
    
    string method callPure(const std::string &name)
    {
        map<string, string> headers = buildHeaders();
        // mobile user agent is not ok here...
        headers["User-Agent"] =  "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_2) AppleWebKit/537.74.9 (KHTML, like Gecko) Version/7.0.2 Safari/537.74.9";
        auto downloader = HttpDownloader::create("https://play.google.com/music/" + name, headers, "GET", "");
        downloader->waitUntilFinished();
        
        string str;
        auto fail = downloader->failed();
        if (!fail) {
            str = *downloader;
        }
        return str;
    }
    
    shared_ptr<JsonStreamer> method streamingCall(const string &name, const Json::Value &param)
    {
        auto downloader = startDownload(name, param, "GET");

        return shared_ptr<JsonStreamer>(new JsonStreamer(downloader));
    }
    
    void method fixJson(string &str)
    {
        // this is a very slow way to deal with long json data...
        /*
        replaceOccurrences(str, ",,", ",0,");
        replaceOccurrences(str, "[,", "[0,");
        replaceOccurrences(str, ",]", ",0]");
        replaceOccurrences(str, "[]", "0");*/
        
        if (str.size() < 1){
            return;
        }
        
        string original = str;
        str.clear();
        str.reserve(original.size() * 2);
        
        char prev = original.at(0);
        str.push_back(prev);
        for (size_t i = 1 ; i < original.size() ; ++i) {
            char c = original.at(i);
            if (prev == ',' && c == ',') {
                str.push_back('0');
                str.push_back(',');
            } else if (prev == '[' && c == ',') {
                str.push_back('0');
                str.push_back(',');
            } else if (prev == ',' && c == ']') {
                str.push_back('0');
                str.push_back(']');
            } else if (prev == '[' && c == ']') {
                str[str.size()-1] = '0';
            } else {
                str.push_back(c);
            }
            prev = c;
        }
    }
    
    shared_ptr<Json::Value> method call(const string &name, const Json::Value &param)
    {
        auto downloader = startDownload(name, param, "POST");
        downloader->waitUntilFinished();
        
        if (!downloader->failed()) {
            
            string str = *downloader;
            
    #if DEBUG
            //       NSLog(@"data received:%@", [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding ]);
    #endif
            bool pureJson = pureJsonForCall(name);
            if (pureJson) {
                
    #if DEBUG
                //NSLog(@"data: %@", [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding]);
    #endif
                
                fixJson(str);
            }
            
            shared_ptr<Json::Value> pt = shared_ptr<Json::Value>(new Json::Value());
            
            Json::Reader reader;
            bool success = reader.parse(str, *pt);
            if (success) {
                return pt;
            } else {
                
    #ifdef VERBOSE_LOG
                if (result == nil) {
                    NSLog(@"error with request. full response: %@", [self loggableStringFrom:[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding]]);
                }
    #endif
                return shared_ptr<Json::Value>();
            }
            
        } else {
            cout << "no data " << downloader->failed() << endl;
            return shared_ptr<Json::Value>();
        }
        
    }
    
    
    const vector<Gear::WebCookie> & method cookiesToSave() const
    {
        return _cookies;
    }
    
    Gear::WebCookie method cookieForName(const string &name)
    {
    	for (auto it = _cookies.begin() ; it != _cookies.end() ; ++it) {
            if (it->name() == name) {
                return *it;
            }
        }
        return WebCookie();
    }
    
    string method buildUrl(const string &name)
    {
        std::stringstream str;
        if (name == "listen") {
            str << "https://play.google.com/music/";
        } else {
            str << "https://play.google.com/music/services/";
        }
        
        str << name << "?u=0&xt=" << cookieForName("xt").value();
        if (pureJsonForCall(name) && name != "addtoplaylist") {
            str << "&format=jsarray";
        }
        return str.str();
    }
    
    bool method loginWithCookies(const Json::Value &cookiesJson)
    {
        vector<map<string,string>> savedCookies;
        if (cookiesJson.isArray()) {
            transform(cookiesJson.begin(), cookiesJson.end(), back_inserter(savedCookies), [](const Json::Value &json){
                
                map<string,string> cookie;
                auto names = json.getMemberNames();
                for (auto it = names.begin() ; it != names.end() ; ++it) {
                	auto &member = *it;
                    cookie[member] = json[member].asString();
                }
                return cookie;
            });
        }
        
        return loginWithCookies(savedCookies);
    }

    bool method loginWithCookies(vector<map<string,string>> savedCookies)
    {
        // filter out xt... we must decide whether we've been kicked out or something since the last time!
        auto it = remove_if(savedCookies.begin(), savedCookies.end(), [](const map<string,string> &m){
            
            auto foundName = m.find("Name");
            if (foundName != m.end() && foundName->second == "xt") {
                return true;
            }
            return false;
        });
        savedCookies.erase(it, savedCookies.end());
        
        if (_userName.empty()) {
            _userName = "unknown";
        }
        
        _loginState = LoginState::LoginStateLoadingStart;
        
        auto pThis = shared_from_this();
        
        {
            unique_lock<mutex> l(_loginLock);
            _loginCanProceed = false;
            
            Base::MainExecutor::instance().addTask([pThis, savedCookies]{
                
                if (pThis->_webWindow) {
                    pThis->_webWindow.reset();
                }
                
                pThis->_webWindow = IApp::instance()->createWebWindow(pThis);
                
                // simulate iphone for nicer login window
                
                pThis->_userAgent = "Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_0 like Mac OS X; en-us) AppleWebKit/532.9 (KHTML, like Gecko) Version/4.0.5 Mobile/8A293 Safari/6531.22.7";
                pThis->_webWindow->setUserAgent(pThis->_userAgent);
                
                // DEBUG:
#ifdef DEBUG_WEBLOAD
                [self showWebView];
#endif
                
#ifdef VERBOSE_LOG
                NSLog(@"cookies were: %@", [self loggableStringFrom:savedCookies]);
#endif
                for (auto it = savedCookies.begin() ; it != savedCookies.end() ; ++it) {
                	auto properties = *it;
                    pThis->_webWindow->injectCookie(WebCookie(properties));
                }
                
                pThis->_webWindow->loadUrl("https://music.google.com");
                
                // DEBUG
                //[webView setFrame:CGRectMake(0, 0, 400, 400)];
                //[[[[AppDelegate sharedDelegate] window] contentView] addSubview:webView];
            });
            
            while (!_loginCanProceed) {
                _loginCondition.wait(l);
            }
        }
        
        std::cout << "loginstate: " << static_cast<int>(_loginState) << std::endl;
        
        Base::MainExecutor::instance().addTask([pThis]{
            pThis->close();
        });
        
        return _loginState == LoginState::LoginStateLoggedin;
    }
    
    void method close()
    {
#ifndef DEBUG_WEBLOAD
        _webWindow.reset();
#endif
    }
    
    void method setUserName(const string &name)
    {
        _userName = name;
    }
    
    const string & method userName() const
    {
        return _userName;
    }
    
    void method didFailLoad(const string &errorMsg)
    {
        _loginState = LoginState::LoginStateNoLogin;
        _errorMessage = errorMsg;
        
        unique_lock<mutex> l(_loginLock);
        _loginCanProceed = true;
        _loginCondition.notify_all();
    }
    
    void method navigationAction()
    {
        string value = _webWindow->fieldValueForElementId("Email");
        if (!value.empty()) {
            _userName = value;
        }
    }
    
    void method didFinishLoad()
    {
        //NSLog(@"didfinishloadforframe: %@", [sender mainFrameURL]);
        
        // handle errors gracefully
        // case 1: wrong password
        // case 2: totally alien redirect (accounts.google.com)  (autotesting1@gmail.com, automatatesting1)
        // case 3: unsupported country https://play.google.com/music/unsupportedcountry
        switch (_loginState) {
            case LoginState::LoginStateLoadingStart: {
                
                auto currentCookies = _webWindow->cookies();
                bool loggedIn = false;
                for (auto it = currentCookies.begin() ; it != currentCookies.end() ; ++it) {
                    if (it->name() == "xt") {
                        // we are logged in
                        loggedIn = true;
                        break;
                    }
                }
                
                _loginState = LoginState::LoginStateLoggingIn;
                if (loggedIn) {
                    finishLogin();
                } else {
                    
                    showWebView();
                    finishLogin();
                }
                //[emailField setValue:email];
                //[passwordField setValue:password];
                //[signInButton click];
            } break;
            case LoginState::LoginStateLoggingIn:{
                
                finishLogin();
            } break;
            default:
                break;
        }
    }
    
    void method showWebView()
    {
        _webWindow->show(u("Authentication"));
    }
    
    void method finishLogin()
    {
    	//using Base::Logger;
        
    	// Base::Logger::stream() << "login: finishlogin()";
    	
        if (_loginCanProceed) {
            cout << "login process over." << endl;
            return;
        }
        
        //NSLog(@"getting cookies....\n");
        
        _cookies = _webWindow->cookies();
        
        if (cookieForName("xt")) {
            
            //Base::Logger::stream() << "login: got xt!";
            
            // good, we're in
            _loginState = LoginState::LoginStateLoggedin;
            _cookieValidUntil = CHRONO_NS::system_clock::now() + CHRONO_NS::milliseconds(kCookieValidityDuration);
            
        } else {
            
            //Base::Logger::stream() << "login: no xt yet";
            
            if (isWebViewShown()) {
                auto pThis = shared_from_this();
                BackgroundExecutor::instance().addTask([pThis]{
                    CHRONO_NS::milliseconds w(1000);
                    this_thread:sleep_for(w);
                    MainExecutor::instance().addTask([pThis]{
                        pThis->finishLogin();
                    });
                });
                return;
            } else {
                
                _loginState = LoginState::LoginStateNoLogin;
            }
            
        }
        lock_guard<mutex> l(_loginLock);
        _loginCanProceed = true;
        _loginCondition.notify_all();
    }
    
    bool method isWebViewShown()
    {
        return _webWindow->visible();
    }
}
