//
//  SpotifyService.cpp
//  G-Ear Player
//
//  Created by Ági Asztalos on 10/13/13.
//
//

#include "SpotifyService.h"
#include "SpotifySession.h"
#include "IApp.h"
#include "BackgroundExecutor.h"
#include "SessionManager.h"

#ifndef DISABLE_SPOTIFY

namespace Gear
{
#define method SpotifyService::
    
    shared_ptr<SpotifyService> method create(shared_ptr<SessionManager> &sessionManager)
    {
        return shared_ptr<SpotifyService>(new SpotifyService(sessionManager));
    }
    
    method SpotifyService(shared_ptr<SessionManager> &sessionManager) :
        _session(SpotifySession::create(IApp::instance())),
        _sessionManager(sessionManager)
    {
    }
    
    string method title() const
    {
        return "Spotify";
    }
    
    void method autoconnect()
    {
        _state = IService::State::Connecting;
        
        auto pThis = shared_from_this();
        Base::BackgroundExecutor::instance().addTask([pThis]{
            if (pThis->_session->autoconnectSync()) {
            	pThis->connnectSucceeded();
            } else {
                pThis->_state = IService::State::Offline;
            }
        });
    }
    
    void method connnectSucceeded()
    {
    	_state = IService::State::Online;
		_sessionManager->addSession(_session);

		_session->refresh();
    }

    void method connect()
    {
        _state = IService::State::Connecting;
        
        auto pThis = shared_from_this();
        Base::BackgroundExecutor::instance().addTask([pThis]{

        	string user;
        	string pass;
        	IApp::instance()->askForCredentials("Spotify", user, pass);
            
            if (pThis->_session->connectSync(user, pass)) {
            	pThis->connnectSucceeded();
            } else {
                pThis->_state = IService::State::Offline;
                pThis->disconnect();
            }
        });
    }
    
    void method disconnect()
    {
        _state = IService::State::Offline;
        auto pThis = shared_from_this();
        Base::BackgroundExecutor::instance().addTask([pThis]{
            pThis->_session->disconnectSync();
            pThis->_sessionManager->removeSession(pThis->_session);
        });
    }
}
#endif
