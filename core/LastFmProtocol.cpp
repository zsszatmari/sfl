//
//  LastFmSession.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/17/12.
//
//

#include <string>
#include <sstream>
#include "json.h"
#include "LastFmProtocol.h"
#include "MainExecutor.h"
#include "IWebWindow.h"
#include "IApp.h"
#include "HttpDownloader.h"
#include "UrlEncode.h"
#include "md5.h"
#include "RateLimitedExecutor.h"
#include "IExecutor.h"

using std::string;
using std::cout;
using std::endl;
using std::stringstream;
using std::make_pair;

#if TARGET_OS_IPHONE
static const string kApiKey = "63cdf679bd30eeeb8de321955afd1ef0";
static const string kSecret = "8d969f1311f7f41c1c7c776d39359d90";
#else
static const string kApiKey = "374759a0103e1c741bc3357c174e0dab";
static const string kSecret = "bd89f33567afd474055e9a9726f1e1d8";
#endif


static const string kKeyValue = "VALUE";

//static const int kErrorNotAuthorized = 14;

#define JSON 1

namespace Gear
{
#define method LastFmProtocol::
    
    static shared_ptr<Base::IExecutor> createExecutor()
    {
        return shared_ptr<IExecutor>(new RateLimitedExecutor(0.3f));
    }
    
    shared_ptr<IExecutor> method executor()
    {
        static shared_ptr<IExecutor> e = createExecutor();
        return e;
    }
    
    shared_ptr<LastFmProtocol> method create(const string &sessionKey, const string &name)
    {
        return shared_ptr<LastFmProtocol>(new LastFmProtocol(sessionKey, name));
    }
    
    method LastFmProtocol(const string &sessionKey, const string &name) :
        _sessionKey(sessionKey),
        _name(name)
    {
    }
    
    const string & method sessionKey() const
    {
        return _sessionKey;
    }
    
    const string & method name() const
    {
        return _name;
    }
    
    bool method authenticate(string &err)
    {
        if (_unusedAuthToken.empty()) {
            _unusedAuthToken = getTokenWithErr(err);
            if (_unusedAuthToken.empty()) {
                return false;
            }
            string tokenInMain = _unusedAuthToken;
            auto pThis = shared_from_this();
            Base::MainExecutor::instance().addTask([pThis, tokenInMain]{
                
                pThis->_webWindow = IApp::instance()->createWebWindow(pThis);
                
                std::stringstream str;
                str << "http://www.last.fm/api/auth/?api_key=" << kApiKey << "&token=" << tokenInMain;
                pThis->_webWindow->loadUrl(str.str());
                pThis->_webWindow->show("Last.fm");
            });
        }
        
        bool success = getSessionForToken(_unusedAuthToken);
        if (!success) {
            
            err = "Waiting for authorization...";
       
            return false;
        } else {
            _unusedAuthToken = "";
        }
        return true;
    }
    
    bool method loggedIn() const
    {
        return !_sessionKey.empty();
    }
    
    static Json::Value call(const string &cmd, const map<string,string> &aParams, bool post, string &err, bool forceHttps = false)
    {
        map<string,string> params = aParams;
        params.insert(make_pair("method", cmd));
        params.insert(make_pair("api_key", kApiKey));
#ifdef JSON
        params.insert(make_pair("format","json"));
#endif
        
        stringstream signature;
        // alphabetically!
        for (auto it = params.begin() ; it != params.end() ; ++it) {
            if (it->first == "format") {
                continue;
            }
            signature << it->first;
            signature << it->second;
        }
        signature << kSecret;
        
        stringstream url;
        // do everything on https
        if (/*forceHttps*/ true) {
            url << "https://ws.audioscrobbler.com/2.0/";
        } else {
            url << "http://ws.audioscrobbler.com/2.0/";
        }
        if (!post) {
            url << "?";
        }
        stringstream paramsToSend;
        for (auto it = params.begin() ; it != params.end() ; ++it) {
            paramsToSend << it->first << "=" << UrlEncode::encode(it->second) << "&";
//            paramsToSend << it->first << "=" << it->second << "&";
        }
        
        /*const char *utfSignature = signature.str().c_str();
        unsigned char hashedSignature[CC_MD5_DIGEST_LENGTH];
        CC_MD5(utfSignature, (CC_LONG)strlen(utfSignature), hashedSignature);
        stringstream hexaSignature;
        for (int i = 0; i < CC_MD5_DIGEST_LENGTH; ++i) {
            hexaSignature << std::hex << std::setw(2) << static_cast<int>(hashedSignature[i]);
            //[hexaSignature appendFormat:@"%02x", hashedSignature[i]];
        }*/
        string hexaSignature = md5(signature.str());
        
        paramsToSend << "api_sig=" << hexaSignature;
        
        shared_ptr<IDownloader> downloader;
        if (!post) {
            url << paramsToSend.str();
            downloader = HttpDownloader::create(url.str());
        } else {
            map<string,string> headers;
            headers.insert(std::make_pair("Content-Type", "application/x-www-form-urlencoded"));
            //headers.push_back(std::pair("Content-Length", ))
            auto body = paramsToSend.str();
            downloader = HttpDownloader::create(url.str(), headers, "POST", body);
        }
#if DEBUG
        cout << "lastfm: " << url.str() << endl;
#endif
        downloader->start();
        downloader->waitUntilFinished();
        if (downloader->failed()) {
            err = "Error connecting";
            return Json::Value();
        }
        
        string data = *downloader;
        
#if DEBUG
        //cout << "lastfm return: " << data << endl;
#endif
        
#ifndef JSON
        // json does not work for whatever reason, sticking to xml
        NSXMLParser *parser = [[NSXMLParser alloc] initWithData:data];
        ParseDictionary *delegate = [[ParseDictionary alloc] init];
        [parser setDelegate:delegate];
        [parser parse];
        if ([delegate error] != nil) {
            if (errReturn != nil) {
                *errReturn = [delegate error];
            }
            return nil;
        }
        
        NSDictionary *ret = [[[delegate dictionary] objectForKey:@"lfm"] objectAtIndex:0];
        NSString *status = [ret objectForKey:@"status"];
        if (![status isEqualToString:@"ok"]) {
            NSDictionary *errorMessage = [[ret objectForKey:@"error"] objectAtIndex:0];
            if (errReturn != nil) {
                *errReturn = [[NSError alloc] initWithDomain:kLastFmErrorDomain code:[[errorMessage objectForKey:@"code"] intValue] userInfo:[NSDictionary dictionaryWithObject:[errorMessage objectForKey:kKeyValue] forKey:NSLocalizedDescriptionKey]];
            }
            return nil;
        }
        
#else
        
        Json::Value ret;
        Json::Reader parser;
        bool success = parser.parse(data, ret);
        
        if (!success) {
            err = "Error in response";
            return ret;
        }
        
        Json::Value errorCode = ret.get("error", Json::nullValue);
        if (errorCode != Json::nullValue) {
        
            string message = ret.get("message", "").asString();
            if (!message.empty()) {
                err = message;
            } else {
                err = errorCode.asString();
            }
            return Json::Value();
        }
#endif
        
        return ret;
    }

    bool method authenticateMobile(string &err, const std::string &user, const std::string &pass)
    {
        map<string,string> params;
        params.insert(std::make_pair("username",user));
        params.insert(std::make_pair("password",pass));

        auto result = call("auth.getMobileSession", params, true, err, true);
        
        if (!err.empty()) {
            return false;
        }
#ifndef JSON
        self.sessionKey = [[[[[result objectForKey:@"session"] objectAtIndex:0]objectForKey:@"key"] objectAtIndex:0] objectForKey:kKeyValue];
        self.name = [[[[[result objectForKey:@"session"] objectAtIndex:0]objectForKey:@"name"] objectAtIndex:0] objectForKey:kKeyValue];
#else
        auto sessionData = result.get("session", Json::objectValue);
        _sessionKey = sessionData.get("key","").asString();
        _name = sessionData.get("name","").asString();
#endif
        
        if (_name.empty() || _sessionKey.empty()) {
            /*if (err != nil) {
                *err = [[NSError alloc] initWithDomain:kLastFmWarningDomain code:kErrorNotAuthorized userInfo:[NSDictionary dictionaryWithObject:@"Error in authorization." forKey:NSLocalizedDescriptionKey]];
            }*/
            err = "Error authenticating";
            return false;
        }
        
        return true;
    }
    
    static Json::Value call(const string &cmd, const map<string,string> &params, string &err)
    {
        return call(cmd, params, false, err);
    }
    
    bool method updateNowPlaying(const map<string, string> &aSongParams)
    {
        //NSAssert(sessionKey != nil, @"no session");
        if (_sessionKey.empty()) {
            return false;
        }
        
        map<string, string> songParams = aSongParams;
        songParams.insert(make_pair("sk", _sessionKey));
        
        string err;
        Json::Value result = call("track.updateNowPlaying", songParams, true, err);
        
        return err.empty();
    }
    
    bool method scrobble(const map<string, string> &aSongParams, string &error)
    {
        map<string, string> songParams = aSongParams;
        songParams.insert(make_pair("sk", _sessionKey));
        string err;
        Json::Value result = call("track.scrobble", songParams, true, err);
        
        return err.empty();
    }

    Json::Value method getAlbumInfo(const map<string,string> &params)
    {
        string err;
        return call("album.getInfo", params, false, err);
    }
    
    Json::Value method getArtistInfo(const map<string,string> &params)
    {
        string err;
        return call("artist.getInfo", params, false, err);
    }
    
    Json::Value method getTrackInfo(const map<string,string> &params)
    {
        string err;
        return call("track.getInfo", params, false, err);
    }
    
    bool method getSessionForToken(const string &token)
    {
        map<string,string> params;
        params.insert(std::make_pair("token",token));
        string err;
        auto result = call("auth.getSession", params, err);
        
        if (!err.empty()) {
            return false;
        }
#ifndef JSON
        self.sessionKey = [[[[[result objectForKey:@"session"] objectAtIndex:0]objectForKey:@"key"] objectAtIndex:0] objectForKey:kKeyValue];
        self.name = [[[[[result objectForKey:@"session"] objectAtIndex:0]objectForKey:@"name"] objectAtIndex:0] objectForKey:kKeyValue];
#else
        auto sessionData = result.get("session", Json::objectValue);
        _sessionKey = sessionData.get("key","").asString();
        _name = sessionData.get("name","").asString();
#endif
        
        if (_name.empty() || _sessionKey.empty()) {
            /*if (err != nil) {
                *err = [[NSError alloc] initWithDomain:kLastFmWarningDomain code:kErrorNotAuthorized userInfo:[NSDictionary dictionaryWithObject:@"Error in authorization." forKey:NSLocalizedDescriptionKey]];
            }*/
            return false;
        }
        
        return true;
    }
    
    void method didFailLoad(const string &errorMessage)
    {
        // todo
    }
    
    void method didFinishLoad()
    {
        // todo
    }
    
    void method navigationAction()
    {
        // todo
    }
    
    bool method controlledCookies() const
    {
        return false;
    }
    
    bool method allowStandaloneBrowser() const
    {
        return true;
    }
    
    string method getTokenWithErr(string &err)
    {
        map<string,string> params;
        Json::Value result = call("auth.getToken",params,err);
        if (!err.empty()) {
            return "";
        }
        
#ifndef JSON
        return [[[result objectForKey:@"token"] objectAtIndex:0] objectForKey:kKeyValue];
#else
        return result.get("token","").asString();
#endif
    }
}

