//
//  GooglePlayService.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#include "GooglePlayService.h"
#include "IApp.h"
#include "GooglePlaySession.h"
#include "IKeychain.h"
#include "SessionManager.h"
#include "BackgroundExecutor.h"
#include "PreferencesPanel.h"
#include "HttpDownloader.h"
#include "IoService.h"

namespace Gear
{
#define method GooglePlayService::
    
    string method identifier()
    {
        return "com.treasurebox.gear.account.googlemusic-cookies";
    }
    
    shared_ptr<GooglePlayService> method create(shared_ptr<SessionManager> &serviceManager)
    {
        shared_ptr<GooglePlayService> ret(new GooglePlayService(serviceManager));
        return ret;
    }
    
    method GooglePlayService(shared_ptr<SessionManager> &sessionManager) :
        _sessionManager(sessionManager)
    {
    }
    
    string method title() const 
    {
        return "Google Music";
    }
    
    void method autoconnect()
    {
    	auto app = IApp::instance();
    	auto keychain = app->keychain();
    	auto username = keychain->username(identifier());
        if (!username.empty()) {
            PreferencesPanel::checkService(*this, [&]{
                connect();
            }, []{
                //std::cout << "nooo\n";
            });
        }
    }
    
    void method connect()
    {
        auto pThis = shared_from_this();
        Io::get().dispatch([pThis]{
            pThis->_state = IService::State::Connecting;
            
            auto googleSession = GooglePlaySession::create(IApp::instance());
            pThis->_activeSession = googleSession;
            pThis->_sessionManager->addSession(googleSession);
            auto user = IApp::instance()->keychain()->username(pThis->identifier());
            bool success = googleSession->connectSync(user, IApp::instance()->keychain()->pass(pThis->identifier()));
        
            if (success) {
                pThis->_state = IService::State::Online;
            } else {
                pThis->_state = IService::State::Offline;
                // don't delete credentials if couldn't connect just because there is no internet
                pThis->disconnect(HttpDownloader::isOnline());
            }
        });
    }
    
    void method disconnect()
    {
        disconnect(true);
    }  

    void method disconnect(bool deleteCredentials)
    {
        auto session = _activeSession.lock();

#ifndef DEBUG_DONTLOGOUT
        if (deleteCredentials) {
            if (session) {
                session->removeData();
            }
            IApp::instance()->keychain()->forget(identifier());
        }
#endif
        _state = IService::State::Offline;
        
        _sessionManager->removeSession(session);
    }

    
    string method inAppIdentifier() const
    {
        return "googlemusic";
    }
    
    bool method inAppNecessary() const
    {
        return true;
    }
}
