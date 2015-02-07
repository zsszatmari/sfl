//
//  YouTubeService.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/23/13.
//
//

#include "YouTubeService.h"
#include "OAuthConnection.h"
#include "YouTubeSession.h"
#include "SessionManager.h"
#include "BackgroundExecutor.h"
#include "IApp.h"
#include "IKeychain.h"
#include "CoreDebug.h"
#include "PreferencesPanel.h"

static const std::string kClientId = "1007807607350.apps.googleusercontent.com";
static const std::string kSecret = "uRn6drhBKkEFpbm5KtxunA8N";

/*
// redirect uri: https://www.localhost.com/oauth2callback
static const std::string kClientId = "1007807607350-6p3ddpe2em5pa85ri5tjglftctln0p5m.apps.googleusercontent.com";
static const std::string kSecret = "gHL8RDpcFJHJZIgLkDpCuufT";
*/


namespace Gear
{
#define method YouTubeService::
    
    static string identifier()
    {
        return "com.treasurebox.gear.account.youtube";
    }
    
    string method title() const
    {
        return "YouTube Personal";
    }
    
    shared_ptr<YouTubeService> method create(shared_ptr<SessionManager> &sessionManager)
    {
        return shared_ptr<YouTubeService>(new YouTubeService(sessionManager));
    }
    
    method YouTubeService(shared_ptr<SessionManager> &sessionManager) :
        _sessionManager(sessionManager)
    {
        resetConnection();
    }
    
    void method resetConnection()
    {
//#ifndef TIZEN
//    	string callback = "http://localhost";
    	string callback = "urn:ietf:wg:oauth:2.0:oob";
//#else
//    	string callback = "http://www.local.com/oauth2callback";
//#endif
        _connection = OAuthConnection::create("https://accounts.google.com/o/oauth2", kClientId, kSecret, "https://www.googleapis.com/auth/youtube", callback, identifier());
    }
    
    void method autoconnect()
    {
        auto success = _connection->autoconnect();
        if (success) {
            PreferencesPanel::checkService(*this, [&]{
                
                _state = IService::State::Online;
                connected();
            }, []{
                //std::cout << "nooo\n";
            });
        }
    }
    
    void method connected()
    {
        _sessionManager->removeSession(_activeSession.lock());
        
        auto session = YouTubeSession::create(IApp::instance(), _connection);
        _activeSession = session;
        _sessionManager->addSession(session);
        
        session->refresh();
    }
    
    void method connect()
    {
        auto pThis = shared_from_this();
        BackgroundExecutor::instance().addTask([pThis]{
            pThis->_state = IService::State::Connecting;
            
            bool success = pThis->_connection->connectSync();
            if (success) {
                pThis->_state = IService::State::Online;
                pThis->connected();
            } else {
                pThis->_state = IService::State::Offline;
                pThis->disconnect();
            }
        });
    }
    
    void method disconnect()
    {
#ifndef DEBUG_DONTLOGOUT
        IApp::instance()->keychain()->forget(identifier());
#endif
        _state = IService::State::Offline;
        
        _sessionManager->removeSession(_activeSession.lock());
        resetConnection();
    }
    
    string method inAppIdentifier() const
    {
        return "youtube";
    }
    
    bool method inAppNecessary() const
    {
        return true;
    }
}
