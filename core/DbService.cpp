//
//  GooglePlayService.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 7/25/13.
//
//

#include <iostream>
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
#define method DbService::
    
    method DbService(const shared_ptr<SessionManager> &sessionManager) :
        _sessionManager(sessionManager)
    {
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
        auto self = shared_from_this();
        Io::get().dispatch([self,this]{
            self->_state = IService::State::Connecting;
            
            auto session = createSession();
            self->_activeSession = session;
            self->_sessionManager->addSession(session);
            auto user = IApp::instance()->keychain()->username(self->identifier());
            
            
#ifdef DEBUG
            std::cout << "trying " << identifier() << " " << session.use_count() << std::endl;
#endif
            session->connect(user, IApp::instance()->keychain()->pass(self->identifier()),
                [self](bool success){
        
                if (success) {
                    self->_state = IService::State::Online;
                } else {
                    self->_state = IService::State::Offline;
                    // don't delete credentials if couldn't connect just because there is no internet
                    self->disconnect(HttpDownloader::isOnline());
                }
            });
#ifdef DEBUG
            std::cout << "try end " << identifier() << " " << session.use_count() << std::endl;
#endif
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
}
