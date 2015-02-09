//
//  AppBase.h
//  G-Ear iOS
//
//  Created by Zsolt Szatmári on 7/8/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_iOS__AppBase__
#define __G_Ear_iOS__AppBase__

#include "IApp.h"
#include "IPlayer.h"
#include "SerialExecutor.h"
#include "DragManager.h"

namespace Gear
{
    class SessionManager;
    class ServiceManager;
    class IAudioDeviceEnumerator;
    class RemoteControlServer;
    class RemoteControl;
    
    class core_export AppBase : public IApp
    {
    public:
        AppBase();
        virtual shared_ptr<SessionManager> sessionManager() const;
        virtual shared_ptr<ServiceManager> serviceManager() const;
        virtual void appStarted();
        virtual shared_ptr<IPlayer> player();
        virtual shared_ptr<PlayerSwitch> playerSwitch();
        virtual shared_ptr<IKeychain> keychain();
        virtual shared_ptr<Gui::ThemeManager> themeManager() const;
        virtual shared_ptr<IPreferencesPanel> preferencesPanel() const;
        
        virtual void playFirstOfCurrentPlaylist(); 
        virtual void showRelated(const SongEntry &entry, const string &baseField);
        virtual DragManager &dragManager();
        virtual SortDescriptor defaultSongOrder() const;

        virtual bool phoneInterface();
        void freeUpMemory();
        virtual shared_ptr<IJavascriptEngine> createJavascriptEngine() const;
        virtual void didBecomeActive();
        void terminate();
        virtual shared_ptr<RemoteControl> remoteControl() const override final;
        
    protected:
        shared_ptr<IKeychain> _keychain;
        
    private:
        DragManager _dragManager;
        
        shared_ptr<SessionManager> _sessionManager;
        shared_ptr<ServiceManager> _serviceManager;
        shared_ptr<Gui::ThemeManager> _themeManager;
        shared_ptr<PlayerSwitch> _player;
        shared_ptr<IPreferencesPanel> _preferencesPanel;
        shared_ptr<RemoteControlServer> _rcServer;
        shared_ptr<RemoteControl> _rcClient;
        
        Base::SerialExecutor _playFirstSheduler;
        
        void playlistsChanged();
        void doPlayFirstOfCurrentPlaylist();

    };
}

#endif /* defined(__G_Ear_iOS__AppBase__) */
