//
//  LastFmController.m
//  G-Ear
//
//  Created by Zsolt Szatmári on 8/17/12.
//
// 

#include <sstream>
#include <math.h>
#include "json.h"
#include "LastFmController.h"
#include "IApp.h"
#include "IKeychain.h"
#include "stdplus.h"
#include CHRONO_H

using namespace boost;
using namespace Gear;


using std::map;

#ifdef _MSC_VER

#define snprintf c99_snprintf

#include <cstdarg>

inline int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

inline int c99_snprintf(char* str, size_t size, const char* format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(str, size, format, ap);
	va_end(ap);

	return count;
}

#endif // _MSC_VER

namespace Gear
{
#define method LastFmController::
    
    static const string kServiceName = "com.treasurebox.gear.lastfm";

    method LastFmController()
    {
    }

    Base::ValueConnector<std::string> method messageConnector()
    {
        return _message.connector();
    }

    Base::ValueConnector<bool> method enabledConnector()
    {
        return _enabled.connector();
    }

    MEMORY_NS::shared_ptr<LastFmController> method create()
    {
        MEMORY_NS::shared_ptr<LastFmController> ret(new LastFmController());
        // trigger check
        ret->enabled();
        return ret;
    }
    
    string method sessionKey()
    {
        // keychain is not yet available upon creation from xib
        auto app = IApp::instance();
        if (app) {
            auto keychain = app->keychain();
            _name = keychain->username(kServiceName);
            return keychain->pass(kServiceName);
        } else {
            return "";
        }
    }

    void method forget()
    {
        IApp::instance()->keychain()->forget(kServiceName);
    }

    void method saveSessionKey(const string &sessionKey, const string &name)
    {
        IApp::instance()->keychain()->save(kServiceName, name, sessionKey);
    }

    bool method enabled()
    {
        string key = sessionKey();
        bool ret = !key.empty();
        if (ret == true && !_protocol) {
            _protocol = LastFmProtocol::create(key, _name);
            setLoggedInMessage();
        }
        _enabled = ret;
        return ret;
    }

    void method setEnabled(bool aEnabled)
    {
        if (!aEnabled) {
            _enabled = false;
            _message = "";
            forget();
            _protocol.reset();
            _message = "";
            return;
        }
        
        if (_enabled == aEnabled) {
            return;
        }
        _enabled = aEnabled;
        if (!_enabled) {
            
            _message = "";
            forget();
            _protocol.reset();
            _message = "";
            return;
        }
        
        _message = "Connecting...";
        
        _protocol = LastFmProtocol::create("", "");

        authenticate();
    }

    void method loginMobile(const string &user, const string &pass)
    {
        _message = "Connecting...";
        _protocol = LastFmProtocol::create("", user);
        auto self = shared_from_this();
        LastFmProtocol::executor()->addTask([self,this,user,pass]{
            string err;
            bool success = _protocol->authenticateMobile(err, user, pass);
            _name = _protocol->name();
            string sessionKey = _protocol->sessionKey();

            if (!success) {
                _message = err;
                _enabled = false;
            } else {
                saveSessionKey(sessionKey, _name);
                setLoggedInMessage();
                _enabled = true;
            }
        });
    }

    void method applicationDidBecomeActive()
    {
#if !TARGET_OS_IPHONE
        if (_enabled) {
            authenticate();
        }
#endif
    }

    void method authenticate()
    {
        if (_protocol && !_protocol->sessionKey().empty()) {
            return;
        }
        auto self = shared_from_this();
        LastFmProtocol::executor()->addTask([self,this]{
            string err;
            bool success = _protocol->authenticate(err);
            _name = _protocol->name();
            string sessionKey = _protocol->sessionKey();

            if (!success) {
                _message = err;
            } else {
                saveSessionKey(sessionKey, _name);
                setLoggedInMessage();
            }
        });
    }
    
    void method setLoggedInMessage()
    {
        std::stringstream ss;
        ss << "Logged in as " << _name;
        _message = ss.str();
    }
    
    static void addToDictionary(std::map<string,string> &dictionary, const std::string &key, const std::string &object, bool required)
    {
        if (!object.empty()) {
            dictionary.insert(std::make_pair(key, object));
        } else if (required) {
            dictionary.insert(std::make_pair(key,""));
        }
    }
    
    static void addToDictionary(std::map<string,string> &dictionary, const std::string &key, uint64_t object)
    {
        if (object > 0) {
            std::stringstream ss;
            ss << object;
            dictionary.insert(std::make_pair(key, ss.str()));
        }
    }
    
    static void addSong(const MEMORY_NS::shared_ptr<ISong> &songToAdd, std::map<string,string> &params)
    {
        auto length = songToAdd->durationMillis() / 1000;
        
        addToDictionary(params, "artist", songToAdd->artist(), true);
        addToDictionary(params, "track", songToAdd->title(), true);
        addToDictionary(params, "album", songToAdd->album(), false);
        addToDictionary(params, "trackNumber", songToAdd->track());
        addToDictionary(params, "duration", length);
        if (!songToAdd->albumArtist().empty() && songToAdd->artist() != songToAdd->albumArtist()) {
            
            addToDictionary(params, "albumArtist", songToAdd->albumArtist(), true);
        }
    }

    void method songBegan(const MEMORY_NS::shared_ptr<ISong> &aSong)
    {
        _song = aSong;
        _songStarted = CHRONO_NS::system_clock::now();
        _scrobbled = false;
        if (!_protocol || !_protocol->loggedIn()) {
            return;
        }
        
        if (!_song) {
            return;
        }
        float length = _song->durationMillis() / 1000.0f;
        if (/*_song->artist().empty() ||*/ _song->title().empty() || length < 30.0f) {
            // can't update with no (namable) song
            _song = nullptr;
            
            return;
        }
        
        
        std::map<string, string> params;
        addSong(aSong, params);

        auto protocol = _protocol;
        LastFmProtocol::executor()->addTask([protocol, params]{
            //NSError *err = nil;
            protocol->updateNowPlaying(params);
        });
        _timeLeftToScrobble = length / 2;
        const float kMaxTime = 4*60.0f;
        if (_timeLeftToScrobble > kMaxTime) {
            _timeLeftToScrobble = kMaxTime;
        }
        // DEBUG
        //timeLeftToScrobble = 10;
    }

    void method playTimePassed(float time)
    {
        if (_protocol && _song && _timeLeftToScrobble > 0 && !_scrobbled) {
            
            auto lastTimeLeftToScrobble = floor(_timeLeftToScrobble);
            _timeLeftToScrobble -= time;
            if (floor(_timeLeftToScrobble) != lastTimeLeftToScrobble) {
                scrobbleIfNeeded();
            }
        }
    }

    static bool lotOfSpace()
    {
#if TARGET_OS_IPHONE
        return true;
#endif
        return false;
    }
    
    void method scrobbleIfNeeded()
    {
        if (!_protocol) {
            _message = "";
            return;
        }
        if (!_protocol->loggedIn()) {
            return;
        }
        if (!_song) {
            setLoggedInMessage();
            return;
        }
        if (_scrobbled) {
            _message = _scrobbledMessage;
            return;
        }
        if (_timeLeftToScrobble <= 0) {
            std::map<string,string> params;
            addSong(_song, params);

            std::stringstream ss;
            ss << (int)CHRONO_NS::duration_cast<CHRONO_NS::seconds>(_songStarted.time_since_epoch()).count();
            params.insert(std::make_pair("timestamp", ss.str()));
            
            auto self = shared_from_this();
            LastFmProtocol::executor()->addTask([params,self,this]{
                string err;
                bool success = _protocol->scrobble(params, err);
                
                _scrobbled = true;
                if (success) {
                    std::stringstream ss;
                    if (lotOfSpace()) {
                        ss << "Logged in as " << _name << "\n\n";
                    }
                    
                    ss << (std::string)_song->artist() << " - " << (std::string)_song->title();
                    
                    if (lotOfSpace()) {
                        ss << "\n";
                    }
                    ss << "\nscrobbled";
                    _scrobbledMessage = ss.str();
                    
                } else {
                    _scrobbledMessage = err;
                }
                _message = _scrobbledMessage;
            });
        } else {
            char timestr[32];
            int32_t seconds = _timeLeftToScrobble;
            snprintf(timestr, sizeof(timestr), "%02d:%02d", seconds / 60, seconds % 60);
            
            std::stringstream ss;
            if (lotOfSpace()) {
                ss << "Logged in as " << _name << "\n\n";
            }
            ss << (std::string)_song->artist() << " - " << (std::string)_song->title() << "\n";
            if (lotOfSpace()) {
                ss << "\n";
            }
            ss << timestr << " left until scrobbled";
            _message = ss.str();
        }
    }
}
